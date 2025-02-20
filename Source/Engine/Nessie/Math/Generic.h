// Generic.h
#pragma once
#include <cmath>
#include "Core/Generic/Concepts.h"

namespace nes::math
{
    template <FloatingPointType Type = float>
    constexpr Type Pi()
    {
        return static_cast<Type>(3.141592653589793238462643383279502884197169399);
    }

    template <FloatingPointType Type = float>
    constexpr Type TwoPi()
    {
        return static_cast<Type>(6.283185307179586476925286766559005768394338798);
    }

    template <FloatingPointType Type = float>
    constexpr Type InversePi()
    {
        return static_cast<Type>(0.318309886183790671537767526745028724068919291480912897495334688);
    }

    template <FloatingPointType Type = float>
    constexpr Type InverseTwoPi()
    {
        return static_cast<Type>(0.159154943091895335768883763372514362034459645740456448747667344);
    }

    template <FloatingPointType Type = float>
    constexpr Type InverseFourPi()
    {
        return static_cast<Type>(0.079577471545947667884441881686257181017229822870228224373833672);
    }

    template <FloatingPointType Type = float>
    constexpr Type PiOverTwo()
    {
        return static_cast<Type>(1.570796326794896619231321691639751442098584699687552910487472296);
    }

    template <FloatingPointType Type = float>
    constexpr Type PiOverFour()
    {
        return static_cast<Type>(0.785398163397448309615660845819875721049292349843776455243736148);
    }

    template <FloatingPointType Type = float>
    constexpr Type SqrtTwo()
    {
        return static_cast<Type>(1.414213562373095048801688724209698078569671875376948073176679737);
    }

    template <FloatingPointType Type = float>
    constexpr Type PrecisionDelta()
    {
        return static_cast<Type>(0.0001);
    }

    template <FloatingPointType Type = float>
    constexpr Type EulersNumber()
    {
        return static_cast<Type>(2.7182818284590452353602874713526624977572);
    }

    template <FloatingPointType Type = float>
    constexpr Type RadiansToDegrees()
    {
        return static_cast<Type>(180.0 / Pi());
    }

    template <FloatingPointType Type = float>
    constexpr Type DegreesToRadians()
    {
        return static_cast<Type>(Pi() / 180.0);
    }

    template <FloatingPointType Type = float>
    constexpr Type Infinity() 
    {
        return std::numeric_limits<Type>::infinity();
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Check to see if two floating point values are 'approximately' equal.
    ///		@param left : Left hand value.
    ///		@param right : Right hand value.
    ///		@param closeEnough : How close do the values need to be to be considered equal? Default is equal to PrecisionDelta().
    //-----------------------------------------------------------------------------------------------------------------------------
    template <FloatingPointType Type = float>
    constexpr bool CheckEqualFloats(const Type left, const Type right, const float closeEnough = PrecisionDelta())
    {   
        const auto closeConversion = static_cast<Type>(closeEnough);
        return std::abs(left - right) < closeConversion || std::abs(right - left) < closeConversion;
    }

    template <FloatingPointType Type = float>
    constexpr Type ToDegrees(const Type radians)
    {
        return radians * RadiansToDegrees<Type>();
    }

    template <FloatingPointType Type = float>
    constexpr Type ToRadians(const Type degrees)
    {
        return degrees * DegreesToRadians<Type>();
    }

    template <FloatingPointType Type = float>
    Type Log2(Type value)
    {
        return std::log2(value);
    }

    template <FloatingPointType Type = float>
    constexpr Type Round(const Type value)
    {
        return std::round(value);
    }

    template <ScalarType ReturnType, FloatingPointType Type>
    ReturnType FloorTo(Type value)
    {
        return static_cast<ReturnType>(std::floor(value));
    }

    template <ScalarType ReturnType, FloatingPointType Type>
    ReturnType CeilTo(Type value)
    {
        return static_cast<ReturnType>(std::ceil(value));
    }

    template <ScalarType Type>
    constexpr Type Clamp(const Type value, const Type min, const Type max)
    {
        if (value < min)
        {
            return min;
        }

        if (value > max)
        {
            return max;
        }

        return value;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Clamps a value to be between 0 and 1.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type ClampNormalized(const Type type)
    {
        return Clamp(type, static_cast<Type>(0.0), static_cast<Type>(1.0));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Clamps a value to be between -1 and 1.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type ClampSignedNormalized(const Type type)
    {
        return Clamp(type, static_cast<Type>(-1.0), static_cast<Type>(1.0));
    }

    template <FloatingPointType Type>
    constexpr Type Lerp(const Type a, const Type b, const Type t)
    {
        return a + t * (b - a);
    }

    template <ScalarType Type>
    constexpr Type Min(const Type a, const Type b)
    {
        return a < b ? a : b;
    }

    template <ScalarType Type>
    constexpr Type Max(const Type a, const Type b)
    {
        return a > b ? a : b;
    }

    template <ScalarType Type>
    constexpr Type Abs(const Type value)
    {
        return value < 0 ? -value : value;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the square root of the value, clamping the value to 0 if it is negative.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type = float>
    Type SafeSqrt(Type value)
    {
        return std::sqrt(Max(value, static_cast<Type>(0.0)));
    }

    template <ScalarType Type>
    constexpr Type Squared(const Type value)
    {
        return value * value;
    }

    template <ScalarType Type>
    constexpr Type Cubed(const Type value)
    {
        return value * value * value;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Raise a value to a power.
    ///		@tparam Type : Must be a floating or integral type.
    ///		@param value : Value to raise to a power.
    ///		@param exponent : Exponent to raise the value to.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type Power(const Type value, const uint32_t exponent)
    {
        if (exponent == 0)
        {
            return static_cast<Type>(1.0);
        }

        Type result = value;
        for (uint32_t i = 1; i < exponent; ++i)
        {
            result *= value;
        }
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Raise a value to a power. This version is only for floating point values and allows
    ///             for negative exponents.
    ///		@tparam Type : Floating point type.
    ///		@param value : Value to raise to a power.
    ///		@param exponent : Exponent to raise the value to.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    constexpr Type PowerF(const Type value, const int exponent)
    {
        if (exponent == 0)
        {
            return static_cast<Type>(1.0);
        }

        if (exponent < 0)
        {
            return 1.0 / Power(value, -exponent);
        }

        Type result = value;
        for (int i = 1; i < exponent; ++i)
        {
            result *= value;
        }
        return result;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      Source: https://stackoverflow.com/questions/14294659/compile-time-constexpr-float-modulo
    // 
    ///		@brief : Modulo operator for floating point values.
    ///		@tparam Type : Floating point type to operate on.
    ///		@param value : Value to mod.
    ///		@param mod : Mod value.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type = float>
    constexpr Type ModF(const Type value, const Type mod)
    {
        return value - static_cast<Type>(static_cast<long long>(value / mod)) * mod;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Generic SmoothStep function that allows higher order smooth steps, from 1 to 6.
    ///             - Higher Orders are more computationally expensive.
    ///             - Default is 2, which is "smoother step". See more orders here: https://en.wikipedia.org/wiki/Smoothstep 
    ///		@tparam Order : Order of the smooth step function. Must be between 1 and 6. 
    ///		@param x : Value to step. Must be between 0 and 1. (It will be clamped).
    //----------------------------------------------------------------------------------------------------
    template <int Order = 2>
    constexpr float SmoothStep(float x)
    {
        static_assert(Order >= 1 && Order <= 6, "Order must be between 1 and 6, inclusive.");

        x = Clamp(x, 0.0f, 1.0f);

        // Smooth Step
        if constexpr (Order == 1)
        {
            return (-2.f * x * x * x) + (3.f * x * x);
        }

        // Smoother Step
        else if constexpr (Order == 2)
        {
            return x * x * x * (x * (x * 6.f - 15.f) + 10.f);
        }

        else if constexpr (Order == 3)
        {
            return -20.f * Power(x, 7) + 70.f * Power(x, 6) - 84.f * Power(x, 5) + 35.f * Power(x, 4);
        }

        else if constexpr (Order == 4)
        {
            return 70.f * Power(x, 9) - 315.f * Power(x, 8) + 540.f * Power(x, 7) - 420.f * Power(x, 6) + 126.f * Power(x, 5);
        }

        else if constexpr (Order == 5)
        {
            return -252.f * Power(x, 11) + 1386.f * Power(x, 10) - 3080.f * Power(x, 9) + 3465.f * Power(x, 8) - 1980.f * Power(x, 7) + 462.f * Power(x, 6);
        }

        else // (Order == 6)
        {
            return 924.f * Power(x, 13) - 6006.f * Power(x, 12) + 16380.f * Power(x, 11) - 24024.f * Power(x, 10) + 20020.f * Power(x, 9) - 9009.f * Power(x, 8) + 1716.f * Power(x, 7);
        }
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the arc sine of the value, clamping the value to be between -1 and 1, to ensure a
    ///        valid result.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type SafeASin(Type value)
    {
        return std::asin(ClampSignedNormalized(value));
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the arc cosine of the value, clamping the value to be between -1 and 1, to ensure a
    ///        valid result.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    Type SafeACos(Type value)
    {
        return std::acos(ClampSignedNormalized(value));
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Inverse of the 3rd order smooth step function. From wikipedia: The inverse of SmoothStep()
    ///             can be useful when doing certain operations in computer graphics when its effect 
    ///             to be reversed or compensated for.
    //----------------------------------------------------------------------------------------------------
    inline float InverseSmoothStep(const float x)
    {
        return 0.5f - std::sin(SafeASin(1.f - 2.f * x) / 3.f);
    }

    bool IsNan(const FloatingPointType auto value)
    {
        return std::isnan(value);
    }

    bool IsInf(const FloatingPointType auto value)
    {
        return std::isinf(value);
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if both a and B are the same sign. In the case where either or both a & b are
    ///             zero, this will return true. Zero is both positive and negative.
    //----------------------------------------------------------------------------------------------------
    template <SignedScalarType Type>
    bool SameSign(const Type a, const Type b)
    {
        static constexpr Type kZero = static_cast<Type>(0);
        
        if constexpr (FloatingPointType<Type>)
        {
            return !(!CheckEqualFloats(a, kZero)
                && !CheckEqualFloats(b, kZero)
                && a * b < kZero);
        }

        else
        {
            return !((a | b) != kZero && (a ^ b) < kZero);
        }
    }

    /// Array of the first 1000 prime numbers.
    constexpr int kPrimes[1000] = 
    {
        2, 3, 5, 7, 11,
        13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101,
        103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191,
        193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
        283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389,
        397, 401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491,
        499, 503, 509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607,
        613, 617, 619, 631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719,
        727, 733, 739, 743, 751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829,
        839, 853, 857, 859, 863, 877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953,
        967, 971, 977, 983, 991, 997, 1009, 1013, 1019, 1021, 1031, 1033, 1039, 1049, 1051,
        1061, 1063, 1069, 1087, 1091, 1093, 1097, 1103, 1109, 1117, 1123, 1129, 1151, 1153,
        1163, 1171, 1181, 1187, 1193, 1201, 1213, 1217, 1223, 1229, 1231, 1237, 1249, 1259,
        1277, 1279, 1283, 1289, 1291, 1297, 1301, 1303, 1307, 1319, 1321, 1327, 1361, 1367,
        1373, 1381, 1399, 1409, 1423, 1427, 1429, 1433, 1439, 1447, 1451, 1453, 1459, 1471,
        1481, 1483, 1487, 1489, 1493, 1499, 1511, 1523, 1531, 1543, 1549, 1553, 1559, 1567,
        1571, 1579, 1583, 1597, 1601, 1607, 1609, 1613, 1619, 1621, 1627, 1637, 1657, 1663,
        1667, 1669, 1693, 1697, 1699, 1709, 1721, 1723, 1733, 1741, 1747, 1753, 1759, 1777,
        1783, 1787, 1789, 1801, 1811, 1823, 1831, 1847, 1861, 1867, 1871, 1873, 1877, 1879,
        1889, 1901, 1907, 1913, 1931, 1933, 1949, 1951, 1973, 1979, 1987, 1993, 1997, 1999,
        2003, 2011, 2017, 2027, 2029, 2039, 2053, 2063, 2069, 2081, 2083, 2087, 2089, 2099,
        2111, 2113, 2129, 2131, 2137, 2141, 2143, 2153, 2161, 2179, 2203, 2207, 2213, 2221,
        2237, 2239, 2243, 2251, 2267, 2269, 2273, 2281, 2287, 2293, 2297, 2309, 2311, 2333,
        2339, 2341, 2347, 2351, 2357, 2371, 2377, 2381, 2383, 2389, 2393, 2399, 2411, 2417,
        2423, 2437, 2441, 2447, 2459, 2467, 2473, 2477, 2503, 2521, 2531, 2539, 2543, 2549,
        2551, 2557, 2579, 2591, 2593, 2609, 2617, 2621, 2633, 2647, 2657, 2659, 2663, 2671,
        2677, 2683, 2687, 2689, 2693, 2699, 2707, 2711, 2713, 2719, 2729, 2731, 2741, 2749,
        2753, 2767, 2777, 2789, 2791, 2797, 2801, 2803, 2819, 2833, 2837, 2843, 2851, 2857,
        2861, 2879, 2887, 2897, 2903, 2909, 2917, 2927, 2939, 2953, 2957, 2963, 2969, 2971,
        2999, 3001, 3011, 3019, 3023, 3037, 3041, 3049, 3061, 3067, 3079, 3083, 3089, 3109,
        3119, 3121, 3137, 3163, 3167, 3169, 3181, 3187, 3191, 3203, 3209, 3217, 3221, 3229,
        3251, 3253, 3257, 3259, 3271, 3299, 3301, 3307, 3313, 3319, 3323, 3329, 3331, 3343,
        3347, 3359, 3361, 3371, 3373, 3389, 3391, 3407, 3413, 3433, 3449, 3457, 3461, 3463,
        3467, 3469, 3491, 3499, 3511, 3517, 3527, 3529, 3533, 3539, 3541, 3547, 3557, 3559,
        3571, 3581, 3583, 3593, 3607, 3613, 3617, 3623, 3631, 3637, 3643, 3659, 3671, 3673,
        3677, 3691, 3697, 3701, 3709, 3719, 3727, 3733, 3739, 3761, 3767, 3769, 3779, 3793,
        3797, 3803, 3821, 3823, 3833, 3847, 3851, 3853, 3863, 3877, 3881, 3889, 3907, 3911,
        3917, 3919, 3923, 3929, 3931, 3943, 3947, 3967, 3989, 4001, 4003, 4007, 4013, 4019,
        4021, 4027, 4049, 4051, 4057, 4073, 4079, 4091, 4093, 4099, 4111, 4127, 4129, 4133,
        4139, 4153, 4157, 4159, 4177, 4201, 4211, 4217, 4219, 4229, 4231, 4241, 4243, 4253,
        4259, 4261, 4271, 4273, 4283, 4289, 4297, 4327, 4337, 4339, 4349, 4357, 4363, 4373,
        4391, 4397, 4409, 4421, 4423, 4441, 4447, 4451, 4457, 4463, 4481, 4483, 4493, 4507,
        4513, 4517, 4519, 4523, 4547, 4549, 4561, 4567, 4583, 4591, 4597, 4603, 4621, 4637,
        4639, 4643, 4649, 4651, 4657, 4663, 4673, 4679, 4691, 4703, 4721, 4723, 4729, 4733,
        4751, 4759, 4783, 4787, 4789, 4793, 4799, 4801, 4813, 4817, 4831, 4861, 4871, 4877,
        4889, 4903, 4909, 4919, 4931, 4933, 4937, 4943, 4951, 4957, 4967, 4969, 4973, 4987,
        4993, 4999, 5003, 5009, 5011, 5021, 5023, 5039, 5051, 5059, 5077, 5081, 5087, 5099,
        5101, 5107, 5113, 5119, 5147, 5153, 5167, 5171, 5179, 5189, 5197, 5209, 5227, 5231,
        5233, 5237, 5261, 5273, 5279, 5281, 5297, 5303, 5309, 5323, 5333, 5347, 5351, 5381,
        5387, 5393, 5399, 5407, 5413, 5417, 5419, 5431, 5437, 5441, 5443, 5449, 5471, 5477,
        5479, 5483, 5501, 5503, 5507, 5519, 5521, 5527, 5531, 5557, 5563, 5569, 5573, 5581,
        5591, 5623, 5639, 5641, 5647, 5651, 5653, 5657, 5659, 5669, 5683, 5689, 5693, 5701,
        5711, 5717, 5737, 5741, 5743, 5749, 5779, 5783, 5791, 5801, 5807, 5813, 5821, 5827,
        5839, 5843, 5849, 5851, 5857, 5861, 5867, 5869, 5879, 5881, 5897, 5903, 5923, 5927,
        5939, 5953, 5981, 5987, 6007, 6011, 6029, 6037, 6043, 6047, 6053, 6067, 6073, 6079,
        6089, 6091, 6101, 6113, 6121, 6131, 6133, 6143, 6151, 6163, 6173, 6197, 6199, 6203,
        6211, 6217, 6221, 6229, 6247, 6257, 6263, 6269, 6271, 6277, 6287, 6299, 6301, 6311,
        6317, 6323, 6329, 6337, 6343, 6353, 6359, 6361, 6367, 6373, 6379, 6389, 6397, 6421,
        6427, 6449, 6451, 6469, 6473, 6481, 6491, 6521, 6529, 6547, 6551, 6553, 6563, 6569,
        6571, 6577, 6581, 6599, 6607, 6619, 6637, 6653, 6659, 6661, 6673, 6679, 6689, 6691,
        6701, 6703, 6709, 6719, 6733, 6737, 6761, 6763, 6779, 6781, 6791, 6793, 6803, 6823,
        6827, 6829, 6833, 6841, 6857, 6863, 6869, 6871, 6883, 6899, 6907, 6911, 6917, 6947,
        6949, 6959, 6961, 6967, 6971, 6977, 6983, 6991, 6997, 7001, 7013, 7019, 7027, 7039,
        7043, 7057, 7069, 7079, 7103, 7109, 7121, 7127, 7129, 7151, 7159, 7177, 7187, 7193,
        7207, 7211, 7213, 7219, 7229, 7237, 7243, 7247, 7253, 7283, 7297, 7307, 7309, 7321,
        7331, 7333, 7349, 7351, 7369, 7393, 7411, 7417, 7433, 7451, 7457, 7459, 7477, 7481,
        7487, 7489, 7499, 7507, 7517, 7523, 7529, 7537, 7541, 7547, 7549, 7559, 7561, 7573,
        7577, 7583, 7589, 7591, 7603, 7607, 7621, 7639, 7643, 7649, 7669, 7673, 7681, 7687,
        7691, 7699, 7703, 7717, 7723, 7727, 7741, 7753, 7757, 7759, 7789, 7793, 7817, 7823,
        7829, 7841, 7853, 7867, 7873, 7877, 7879, 7883, 7901, 7907, 7919
    };
}