#version 450

layout(binding = 0) uniform Camera
{
    mat4 projectionMatrix;
    mat4 viewMatrix;
};

layout(location = 0) in vec3 nearPoint;
layout(location = 1) in vec3 farPoint;

layout(location = 0) out vec4 outColor;

/// Credit: https://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/
vec4 grid(vec3 fragPos, float scale)
{
    vec2 coord = fragPos.xz * scale;
    vec2 deriv = fwidth(coord);
    vec2 grid = abs(fract(coord - 0.5) - 0.5) / deriv;
    float line = min(grid.x, grid.y);
    float minz = min(deriv.y, 1);
    float minx = min(deriv.x, 1);
    vec4 color = vec4(0.2, 0.2, 0.2, 1.0 - min(line, 1.0));
    if (fragPos.x > -0.5 * minx && fragPos.x < 0.5 * minx)
    color.b = 1.0;
    if (fragPos.z > -0.5 * minz && fragPos.z < 0.5 * minz)
    color.r = 1.0;
    return color;
}

float computeDepth(vec3 fragPos)
{
    vec4 clipPos = vec4(projectionMatrix * viewMatrix * vec4(fragPos, 1.0));
    return clipPos.z / clipPos.w;
}

float computeLinearDepth(vec3 fragPos)
{
    float clipDepth = computeDepth(fragPos) * 2.0 - 1.0;
    float near = 0.1;
    float far = 256;
    float linearDepth = (2.0 * near * far) / (far + near - clipDepth * (far - near));
    return linearDepth / far;
}

void main() 
{
    float t = -nearPoint.y / (farPoint.y - nearPoint.y);
    vec3 fragPos = nearPoint + t * (farPoint - nearPoint);
    outColor = grid(fragPos, 1) * float(t > 0);
    outColor.a *= max(0, (0.5 - computeLinearDepth(fragPos)));

    gl_FragDepth = computeDepth(fragPos);
}