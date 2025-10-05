#version 450

const float kPi = 3.14159265358979323846;
const float kPiOverOne = 1.0 / kPi;
const float kTwoPi = 2.0 * kPi;
const float kEpsilon = 1e-6;
const float kSmallEpsilon = 1e-15;
const float kRf0Dialectrics = 0.04;

const mat4 kShadowBiasMatrix = mat4
(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 1.0, 0.0,
    0.5, 0.5, 0.0, 1.0
);

// Poisson disk samples for smoother shadows with fewer samples
const vec2 poissonDisk[16] = vec2[]
(
    vec2(-0.94201624, -0.39906216),
    vec2(0.94558609, -0.76890725),
    vec2(-0.094184101, -0.92938870),
    vec2(0.34495938, 0.29387760),
    vec2(-0.91588581, 0.45771432),
    vec2(-0.81544232, -0.87912464),
    vec2(-0.38277543, 0.27676845),
    vec2(0.97484398, 0.75648379),
    vec2(0.44323325, -0.97511554),
    vec2(0.53742981, -0.47373420),
    vec2(-0.26496911, -0.41893023),
    vec2(0.79197514, 0.19090188),
    vec2(-0.24188840, 0.99706507),
    vec2(-0.81409955, 0.91437590),
    vec2(0.19984126, 0.78641367),
    vec2(0.14383161, -0.14100790)
);

struct PointLight
{
    vec3 position;
    float intensity;    // Lumens:
    vec3 color;
    float radius;       // Light attenuation radius.
    float falloffExp;   // Falloff exponent (usually 2.0 for physical).
};

struct DirectionalLight
{
    vec3 direction;     // Direction of the Light.
    float intensity;    // Lux (lumens per m^2).
    vec3 color;         // Color
};

#define DEFAULT_SHADOW_AMBIENT 0.025

//-----------------------------------
// Set 0: Camera UBO
//-----------------------------------
layout (set = 0, binding = 0) uniform CameraUBO
{
    mat4 view;
    mat4 proj;
    mat4 viewProj;
    vec3 position;
    float exposureFactor;
} u_camera;

//-----------------------------------
// Set 1: Material UBO:
//-----------------------------------
struct MaterialUBO
{
    vec3  baseColorScale;
    float metallicScale;
    vec3  emissionScale;
    float roughnessScale;
    float normalScale;

    uint  baseColorIndex;
    uint  normalIndex;
    uint  roughnessMetallicIndex;
    uint  emissionIndex;
};

layout (set = 1, binding = 0) readonly buffer MaterialSSBO
{
    MaterialUBO u_materials[];
};

//-----------------------------------
// Set 2: Lighting
//-----------------------------------
layout (set = 2, binding = 0) uniform LightCountUBO
{
    uint directional;
    uint point;
    uint spot;
    uint area;
} u_lightCounts;

layout (set = 2, binding = 1) readonly buffer DirectionalLightSSBO
{
    DirectionalLight u_directionalLights[];
};

layout (set = 2, binding = 2) readonly buffer PointLightSSBO
{
    PointLight u_pointLights[];
};

// [TODO]: Spot Lights, Area Lights.

//-----------------------------------
// Set 3: Shadow Data
//-----------------------------------
layout (set = 3, binding = 0) uniform sampler           u_shadowSampler;
layout (set = 3, binding = 1) uniform texture2DArray    u_shadowMap;
layout (set = 3, binding = 2) uniform ShadowUBO
{
    mat4    cascadeViewProjMatrices[4];
    vec4    splitDepths;
    uint    numCascades;
    float   bias;
} u_shadow;

//-----------------------------------
// Set 4: Sampler and Material Maps
//-----------------------------------
layout (set = 4, binding = 0) uniform sampler       u_sampler;
layout (set = 4, binding = 1) uniform texture2D     u_baseColorMap;
layout (set = 4, binding = 2) uniform texture2D     u_normalMap;
layout (set = 4, binding = 3) uniform texture2D     u_roughnessMetallicMap;
layout (set = 4, binding = 4) uniform texture2D     u_emissionMap;
              
//------------------------------------
// Push Constants
//------------------------------------
layout (push_constant) uniform InstanceData
{
    mat4 modelMatrix;   // Transform vertices from object -> world space.
    mat4 normalMatrix;  // Transform normals/tangents from object -> world space.
    uint meshIndex;
    uint materialIndex;
} u_instance;

// Vertex Input:
layout (location = 0) in vec3 inWorldPosition;
layout (location = 1) in vec3 inWorldNormal;
layout (location = 2) in vec2 inTexCoord;
layout (location = 3) in vec3 inWorldTangent;
layout (location = 4) in vec3 inWorldBitangent;
layout (location = 5) in vec3 inViewPosition;

// Fragment Output:
layout (location = 0) out vec4 outColor;

// Clamps the value between [0, 1].
float Saturate(float value)
{
    return clamp(value, 0.0, 1.0); 
}

// Clamps each componet value between [0, 1].
vec3 Saturate(vec3 value)
{
    return vec3(Saturate(value.x), Saturate(value.y), Saturate(value.z));
}

// Square Root for values between [0, 1]
float Sqrt01(float value)
{
    return sqrt(Saturate(value));
}

// "Positive Reciprocal" (1 / positive)
float PositiveRcp(float value)
{
    return 1.0 / max(value, kSmallEpsilon);
}

float Luminance(vec3 x)
{
    // https://en.wikipedia.org/wiki/Relative_luminance
    return dot(x, vec3( 0.2126f, 0.7152f, 0.0722f ));
}

vec3 Saturation(vec3 x, float amount)
{
    float luma = Luminance(x);
    return mix(x, vec3(luma), amount);
}

void ConvertBaseColorMetalnessToAlbedoRf0(vec3 baseColor, float metalness, out vec3 albedo, out vec3 Rf0)
{
    albedo = baseColor * Saturate(1.0 - metalness);
    Rf0 = mix(vec3(kRf0Dialectrics), baseColor, metalness);
}

vec3 CorrectDirectionToInfiniteSource(vec3 N, vec3 L, vec3 V, float tanOfAngularSize)
{
    vec3 R = reflect(-V, N);
    vec3 centerToRay = L - dot(L, R) * R;
    vec3 closestPoint = centerToRay * Saturate(tanOfAngularSize * inversesqrt(length(centerToRay)));
    
    return normalize(L - closestPoint);
}

//---------------------------------------------------------------------------------------------------------
// Distrubution Terms
//---------------------------------------------------------------------------------------------------------

float DistributionTerm_Blinn(float linearRoughness, float NoH)
{
    float m = Saturate(linearRoughness * linearRoughness);
    float m2 = m * m;
    float alpha = 2.0 * PositiveRcp(m2) - 2.0;
    float norm = (alpha + 2.0) / 2.0;
    float d = norm * pow(Saturate(NoH), alpha);
    
    return d / radians(180.0);
}

float DistributionTerm_GGX(float linearRoughness, float NoH)
{
    float a2 = linearRoughness * linearRoughness;
    float f = (NoH * a2 - NoH) * NoH + 1.0;
    return a2 / (kPi * f * f);
    
    // [NOTE]: The below sharpens the focus of light significantly. The above looks a bit better.
    
    // GGX / Trowbridge-Reitz, [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
    //float alpha = Saturate(linearRoughness * linearRoughness);
    //float alpha2 = alpha * alpha;
    //
    //float t = 1.0 - NoH * NoH * (0.99999994 - alpha2);
    //float a = max(alpha, kEpsilon) / t;
    //float d = a * a;
    //
    //return d / radians(180.0);
}

float DistributionTerm(float linearRoughness, float NoH)
{
    return DistributionTerm_GGX(linearRoughness, NoH);
}

//---------------------------------------------------------------------------------------------------------
// Geometry Terms
//---------------------------------------------------------------------------------------------------------

float GeometryTerm_SmithCorrelated(float linearRoughness, float NoL, float NoV, float VoH, float NoH)
{
    // [Smith 1967, "Geometrical shadowing of a random rough surface"]
    // https://twvideo01.ubm-us.net/o1/vault/gdc2017/Presentations/Hammon_Earl_PBR_Diffuse_Lighting.pdf
    // Known as "G2 height correlated"
    
    float m = Saturate(linearRoughness * linearRoughness);
    float m2 = m * m;
    float a = NoV * Sqrt01((NoL - m2 * NoL) * NoL + m2);
    float b = NoL * Sqrt01((NoV - m2 * NoV) * NoV + m2);
    
    return 0.5 * PositiveRcp(a + b);
}

float GeometryTerm_SchlickGGX(float NoX, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;
    return NoX / (NoX * (1.0 - k) + k);
}

float GeometryTerm_Smith(float linearRoughness, float NoL, float NoV, float VoH, float NoH)
{
    return GeometryTerm_SchlickGGX(NoL, linearRoughness) * GeometryTerm_SchlickGGX(NoV, linearRoughness);
}

float GeometryTerm(float linearRoughness, float NoL, float NoV, float VoH, float NoH)
{
    //return GeometryTerm_SmithCorrelated(linearRoughness, NoL, NoV, VoH, NoH);
    return GeometryTerm_Smith(linearRoughness, NoL, NoV, VoH, NoH);
}

//---------------------------------------------------------------------------------------------------------
// Fresnel Terms
//---------------------------------------------------------------------------------------------------------

vec3 FresnelTerm_Schlick(vec3 Rf0, float VoNH)
{
    return Rf0 + (1.0 - Rf0) * pow(Saturate(1.0 - VoNH), 5.0);
}

float FresnelTerm_Shadowing(vec3 Rf0)
{
    // UE4: anything less than 2% is physically impossible and is instead considered to be shadowing
    return Saturate(Luminance(Rf0) / 0.02f);
}

vec3 FresnelTerm(vec3 Rf0, float VoH)
{
    // FresnelTerm_Schlick
    return FresnelTerm_Schlick(Rf0, VoH) * FresnelTerm_Shadowing(Rf0);
}

//---------------------------------------------------------------------------------------------------------
// Diffuse Terms
//---------------------------------------------------------------------------------------------------------

float DiffuseTerm_Lambert(float linearRoughness, float NoL, float NoV, float VoH)
{
    float d = 1.0;
    return d / Saturate(radians(180.f));
}

float DiffuseTerm_BurleyFrostbite(float linearRoughness, float NoL, float NoV, float VoH)
{
    // [Lagarde 2014, "Moving Frostbite to Physically Based Rendering 3.0"]
    // https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf (page 10)
    
    linearRoughness = Saturate(linearRoughness);
    
    float energyBias = mix(0.0, 0.5, linearRoughness);
    float energyFactor = mix(1.0, 1.0 / 1.51, linearRoughness);
    
    float f = 2.0 * VoH * VoH * linearRoughness + energyBias - 1.0; // Linear Roughness.
    float FdV = 1.0 + f * pow(Saturate(NoV), 5.0);
    float FdL = 1.0 + f * pow(Saturate(NoL), 5.0);
    float d = FdV * FdL * energyFactor;
    
    return d / radians(180.0);
}

float DiffuseTerm(float linearRoughness, float NoL, float NoV, float VoH)
{
    //return DiffuseTerm_Lambert(linearRoughness, NoL, NoV, VoH);
    return DiffuseTerm_BurleyFrostbite(linearRoughness, NoL, NoV, VoH);
}

//---------------------------------------------------------------------------------------------------------
// Environment Terms
//---------------------------------------------------------------------------------------------------------

vec3 EnvironmentTerm(vec3 Rf0, float NoV, float linearRoughness)
{
    // Shlick's approximation for Ross BRDF - makes Fresnel converge to less than 1.0f when NoV is low
    // https://hal.inria.fr/inria-00443630/file/article-1.pdf
    
    float m = Saturate(linearRoughness * linearRoughness);
    float f = pow(Saturate(1.0f - NoV), 5.0 * exp(-2.69 * m)) / (1.0 + 22.7 * pow(Saturate(m), 1.5));
    
    float scale = 1.0 - f;
    float bias = f;
    
    return Saturate(Rf0 * scale + bias);
}

//---------------------------------------------------------------------------------------------------------
// Lighting
//---------------------------------------------------------------------------------------------------------

// Smooth Distance Attenuation for a punctual light.
float SmoothDistanceAtt(float squaredDistance, float invSqrAttRadius)
{
    float factor = squaredDistance * invSqrAttRadius;
    float smoothFactor = Saturate(1.0 - factor * factor);
    return smoothFactor * smoothFactor;
}

// Get the Distance Attenutation for a punctual light.
float GetDistanceAtt(vec3 unormalizedLightVector, float invSqrAttRadius)
{
    float sqrDist = dot(unormalizedLightVector, vec3(invSqrAttRadius));
    float attenuation = 1.0 / (max(sqrDist, 0.01 * 0.01));
    attenuation *= SmoothDistanceAtt(sqrDist, invSqrAttRadius);
    return attenuation;
}

float GetDistanceAtt(float distance, float radius)
{
    float falloff = distance / radius;
    falloff *= falloff;
    falloff = Saturate( 1.0f - falloff * falloff );
    falloff *= falloff;

    float atten = falloff;
    atten *= PositiveRcp(distance * distance + 1.0f);

    return atten;
}

// Calculate photometric distance attenuation
float GetPhotometricAtt(float distance, float radius)
{
    // Inverse square law with smooth cutoff
    float attenuation = 1.0 / (distance * distance);
    
    float cutoff = 1.0 - pow(Saturate(distance / radius), 4.0);
    cutoff * cutoff;
    return attenuation * cutoff;
}

vec3 BRDF(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness, vec3 Rf0)
{
    vec3 H = normalize(L + V);
    float NoV = max(dot(N, V), kEpsilon);
    float NoL = max(dot(N, L), kEpsilon);
    float HoV = max(dot(H, V), 0.0);
    float NoH = max(dot(N, H), 0.0);

    float D = DistributionTerm(roughness, NoH);
    float G = GeometryTerm(roughness, NoL, NoV, HoV, NoH);
    vec3 F = FresnelTerm(Rf0, HoV);

    // Cook-Torrance BRDF
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = D * G * F;
    float denominator = 4.0 * NoV * NoL + kEpsilon;
    vec3 specular = numerator / denominator;

    return (kD * albedo / kPi + specular) * NoL;
}

/*
    Gamma ramps and encoding transfer functions
    Taken from https://github.com/Microsoft/DirectX-Graphics-Samples/blob/master/MiniEngine/Core/Shaders/ColorSpaceUtility.hlsli

    Orthogonal to color space though usually tightly coupled. For instance, sRGB is both a
    color space (defined by three basis vectors and a white point) and a gamma ramp. Gamma
    ramps are designed to reduce perceptual error when quantizing floats to integers with a
    limited number of bits. More variation is needed in darker colors because our eyes are
    more sensitive in the dark. The way the curve helps is that it spreads out dark values
    across more code words allowing for more variation. Likewise, bright values are merged
    together into fewer code words allowing for less variation.

    The sRGB curve is not a true gamma ramp but rather a piecewise function comprising a linear
    section and a power function. When sRGB-encoded colors are passed to an LCD monitor, they
    look correct on screen because the monitor expects the colors to be encoded with sRGB, and it
    removes the sRGB curve to linearize the values. When textures are encoded with sRGB, as many
    are, the sRGB curve needs to be removed before involving the colors in linear mathematics such
    as physically based lighting.
*/

vec3 FromGamma(vec3 x, float gamma)
{
    return pow(Saturate(x), vec3(gamma));
}

vec3 HDRToLinear(vec3 colorMulExposure)
{
    vec3 x0 = colorMulExposure * 0.38317;
    vec3 x1 = FromGamma(1.0 - exp(-colorMulExposure), 2.2);
    vec3 color = mix(x0, x1, step(1.413f, colorMulExposure));

    return color;
}

vec3 GetNormalFromMap(MaterialUBO material)
{
    vec3 tangentNormal = texture(sampler2D(u_normalMap, u_sampler), inTexCoord).xyz * 2.0 - 1.0;
    tangentNormal.xy *= material.normalScale;
    
    mat3 TBN = mat3
    (
        normalize(inWorldTangent),
        normalize(inWorldBitangent),
        normalize(inWorldNormal)
    );
    
    return normalize(TBN * tangentNormal);
}

// Get the Shadow Cascade index for the given position in view space. 
uint GetCascadeIndex(float viewPositionZ)
{
    // ViewPositionZ is negative for objects in front of camera
    // SplitDepths are negative and ordered from near (less negative) to far (more negative)
    uint result = 0;
    
    for (uint i = 0; i < u_shadow.numCascades; ++i)
    {
        if (viewPositionZ < u_shadow.splitDepths[i])
        {
            return i;
        }
    }
    
    return u_shadow.numCascades - 1;
}

// Samples the shadow map given the position in light space.
float GetShadowFactor(vec4 lightSpacePos, vec2 offset, uint cascadeIndex)
{
    float shadow = 1.0;

    // Convert from NDC [-1, 1] to texture coordinates [0, 1]
    vec3 projCoords = lightSpacePos.xyz * 0.5 + 0.5;

    // Clamp to valid texture range
    projCoords.xy = clamp(projCoords.xy, 0.0, 1.0);
    
    // Bounds check:
    if (projCoords.z > 0.0 && projCoords.z < 1.0)
    {
        // Here, the cascade index acts as the layer of the shadow map.
        float dist = texture(sampler2DArray(u_shadowMap, u_shadowSampler), vec3(projCoords.xy + offset, cascadeIndex)).r;

        // If current depth (minus bias) is GREATER than stored depth, it's in shadow
        if (lightSpacePos.w > 0 && lightSpacePos.z - u_shadow.bias > dist)
        {
            shadow = DEFAULT_SHADOW_AMBIENT;
        }
    }
    return shadow;
}

// Samples and averages surrounding texels in a grid pattern to get a smoother transition.  
float GetShadowFactorPCF(vec4 lightSpacePos, uint cascadeIndex)
{
    ivec3 textureDimensions = textureSize(sampler2DArray(u_shadowMap, u_shadowSampler), 0);
    const float scale = 1.0;
    float texelSizeX = scale * 1.0 / float(textureDimensions.x);
    float texelSizeY = scale * 1.0 / float(textureDimensions.y);
    
    // Determines the number of samples to take.
    // [-range -> range] in width and height.
    const int range = 2; 
    
    float shadowFactor = 0.0;
    int count = 0;
    
    // Sample 9 pixels, the center being this pixel
    for (int x = -range; x <= range; ++x)
    {
        for (int y = -range; y <= range; ++y)
        {
            shadowFactor += GetShadowFactor(lightSpacePos, vec2(texelSizeX * x, texelSizeY * y), cascadeIndex);
            count++;
        }
    }
    
    return shadowFactor / count;
}

// Uses a poisson disk to sample the surrounding area of texels to create a softer edge.
float GetShadowFactorPoisson(vec4 lightSpacePos, uint cascadeIndex)
{
    ivec3 textureDimensions = textureSize(sampler2DArray(u_shadowMap, u_shadowSampler), 0);
    const float scale = 1.0;
    vec2 texelSize = vec2(scale * 1.0 / float(textureDimensions.x), scale * 1.0 / float(textureDimensions.y));
    
    float shadowFactor = 0.0;
    const float spread = 3.0; // Adjust this to control shadow softness (1.0 to 5.0)

    // Use the Poisson Disk to sample from
    for (int i = 0; i < 16; ++i)
    {
        vec2 offset = poissonDisk[i] * texelSize * spread;
        shadowFactor += GetShadowFactor(lightSpacePos, offset, cascadeIndex);
    }
    shadowFactor /= 16.0;

    return shadowFactor;
}

float CalculateShadow(vec3 worldPosition, vec3 N, vec3 L, float viewPositionZ)
{
    uint cascadeIndex = GetCascadeIndex(viewPositionZ);
    vec4 lightSpacePos = (u_shadow.cascadeViewProjMatrices[cascadeIndex]) * vec4(worldPosition, 1.0);
    
    // Perform perspective division:
    lightSpacePos.xyz /= lightSpacePos.w;
    
    //float shadow = GetShadowFactor(lightSpacePos, vec2(0.0), cascadeIndex);
    //float shadow = GetShadowFactorPCF(lightSpacePos, cascadeIndex);
    float shadow = GetShadowFactorPoisson(lightSpacePos, cascadeIndex);
    
    return shadow;
}

void main()
{
    MaterialUBO material = u_materials[u_instance.materialIndex];
    
    vec3 V = normalize(u_camera.position - inWorldPosition);
    vec3 N = GetNormalFromMap(material);
    
    // Metallic & Roughness
    vec3 metallicRoughness = texture(sampler2D(u_roughnessMetallicMap, u_sampler), inTexCoord).rgb;
    float metallic = metallicRoughness.b * material.metallicScale;
    float roughness = metallicRoughness.g * material.roughnessScale;

    // Emissive:
    vec3 emissive = texture(sampler2D(u_emissionMap, u_sampler), inTexCoord).rgb * material.emissionScale;

    // Albedo / Rf0.
    vec3 albedo = texture(sampler2D(u_baseColorMap, u_sampler), inTexCoord).rgb * material.baseColorScale;
    vec3 Rf0 = mix(vec3(kRf0Dialectrics), albedo, metallic);
    
    // Calculate Lighting:
    vec3 lightSum = vec3(0.0);

    // DIRECTIONAL LIGHTS
    for (uint i = 0; i < u_lightCounts.directional; ++i)
    {
        DirectionalLight light = u_directionalLights[i];
        vec3 L = -light.direction;
        float NoL = Saturate(dot(N, L));
        
        // Skip if the surface doesn't face the light.
        if (NoL <= 0.0)
            continue;
        
        // Convert illuminance (lux) to radiance
        vec3 radiance = light.color * light.intensity / kPi;
        
        // Calculate Shadow (directional lights are assumed to be shadow casting).
        float shadow = CalculateShadow(inWorldPosition, N, L, inViewPosition.z);
        
        lightSum += BRDF(N, V, L, albedo, metallic, roughness, Rf0) * radiance * (NoL * shadow);
    }
    
    // POINT LIGHTS
    for (uint i = 0u; i < u_lightCounts.point; ++i)
    {
        PointLight light = u_pointLights[i];
        vec3 L = light.position - inWorldPosition;
        float distance = length(L);
        L /= distance;

        float luminousIntensity = light.intensity / (4.0 * kPi);

        float attenuation = GetPhotometricAtt(distance, light.radius);
        vec3 radiance = light.color * luminousIntensity * attenuation;

        lightSum += BRDF(N, V, L, albedo, metallic, roughness, Rf0) * radiance;
    }

    // [TODO]: SPOT LIGHTS

    // [TODO]: AREA LIGHTS

    // [TODO]: IMAGE BASED LIGHTING (IBL) - Environmental Lighting
    // - When you add IBL, replace the simple ambient below with proper environment lighting

    // Sky Ambient Light (separate from directional light)
    //const vec3 skyColor = vec3(0.4, 0.7, 1.0);
    //const float skyIntensity = 8000.0; // lux
    //vec3 ambient = albedo * skyColor * (skyIntensity / kPi);

    // Simple Ambient:
    // This ensures the base color is always visible
    vec3 ambient = albedo * 0.03; // 3% ambient contribution

    // Resulting color = Direct Lighting + Indirect Lighting + Emissive
    vec3 color = lightSum + ambient + emissive;

    // Convert to linear color:
    color = HDRToLinear(color * u_camera.exposureFactor);

    outColor = vec4(color, 1.0);
    
//    // Debug Visualize Cascade Index
//    uint cascadeIndex = GetCascadeIndex(inViewPosition.z);
//    switch (cascadeIndex) 
//    {
//        case 0 :
//            outColor.rgb *= vec3(1.0f, 0.25f, 0.25f);
//            break;
//        case 1 :
//            outColor.rgb *= vec3(0.25f, 1.0f, 0.25f);
//            break;
//        case 2 :
//            outColor.rgb *= vec3(0.25f, 0.25f, 1.0f);
//            break;
//        case 3:
//            outColor.rgb *= vec3(1.0f, 1.0f, 0.25f);
//            break;
//    }
}