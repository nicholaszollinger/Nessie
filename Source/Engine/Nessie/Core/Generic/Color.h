#pragma once
// Color.h
#include <cstdint>

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : RGBA Color represented as integer values from 0 to 255.
    //----------------------------------------------------------------------------------------------------
    struct Color
    {
        uint8_t r = 255;
        uint8_t g = 255;
        uint8_t b = 255;
        uint8_t a = 255;

        constexpr Color() = default;
        constexpr Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255) : r(r), g(g), b(b), a(a) {}

        constexpr bool operator==(const Color& other) const
        {
            return r == other.r && g == other.g && b == other.b && a == other.a;
        }
        
        constexpr bool operator!=(const Color& other) const { return !(*this == other); }

        static constexpr Color White()      { return {}; }
        static constexpr Color Black()      { return {0, 0, 0 }; }
        static constexpr Color Red()        { return { 255, 0, 0 }; }
        static constexpr Color Green()      { return { 0, 255, 0 }; }
        static constexpr Color Blue()       { return { 0, 0, 255 }; }
        static constexpr Color Yellow()     { return { 255, 255, 0 }; }
        static constexpr Color Cyan()       { return { 0, 255, 255 }; }
        static constexpr Color Magenta()    { return { 255, 0, 255 }; }
    };

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : RGBA Color represented as float values from 0.0 to 1.0.
    //----------------------------------------------------------------------------------------------------
    struct LinearColor
    {
        float r = 1.0f;
        float g = 1.0f;
        float b = 1.0f;
        float a = 1.0f;

        constexpr LinearColor() = default;
        constexpr LinearColor(const float r, const float g, const float b, const float a = 1.0f) : r(r), g(g), b(b), a(a) {}

        //constexpr bool operator==(const LinearColor& other) const;
        //constexpr bool operator!=(const LinearColor& other) const { return !(*this == other); }

        static constexpr LinearColor White()    { return {}; }
        static constexpr LinearColor Black()    { return { 0.0f, 0.0f, 0.0f }; }
        static constexpr LinearColor Gray()     { return { 0.5f, 0.5f, 0.5f }; }
        static constexpr LinearColor Red()      { return { 1.0f, 0.0f, 0.0f }; }
        static constexpr LinearColor Green()    { return { 0.0f, 1.0f, 0.0f }; }
        static constexpr LinearColor Blue()     { return { 0.0f, 0.0f, 1.0f }; }
        static constexpr LinearColor Yellow()   { return { 1.0f, 1.0f, 0.0f }; }
        static constexpr LinearColor Cyan()     { return { 0.0f, 1.0f, 1.0f }; }
        static constexpr LinearColor Magenta()  { return { 1.0f, 0.0f, 1.0f }; }
    };

    constexpr Color ToColor(const LinearColor& linearColor)
    {
        return Color
        (
            static_cast<uint8_t>(linearColor.r * 255.0f),
            static_cast<uint8_t>(linearColor.g * 255.0f),
            static_cast<uint8_t>(linearColor.b * 255.0f),
            static_cast<uint8_t>(linearColor.a * 255.0f)
        );
    }

    constexpr LinearColor ToLinearColor(const Color& color)
    {
        return LinearColor
        (
            static_cast<float>(color.r) / 255.0f,
            static_cast<float>(color.g) / 255.0f,
            static_cast<float>(color.b) / 255.0f,
            static_cast<float>(color.a) / 255.0f
        );
    }

    constexpr Color HSVtoRGB(const float hue, const float saturation, const float value)
    {
        // TODO: This was found with a google search for "HSV to RGB" (showed up in the "AI overview"),
        // and I threw it in here to get something working. I need to verify things, and change this
        // to have variable names that make more sense.
        const int i = static_cast<int>(hue * 6);
        const float f = hue * 6 - static_cast<float>(i);
        const float p = value * (1.f - saturation);
        const float q = value * (1.f - f * saturation);
        const float t = value * (1.f - (1.f - f) * saturation);

        switch (i % 6)
        {
            case 0: return ToColor(LinearColor(value, t, p));
            case 1: return ToColor(LinearColor(q, value, p));
            case 2: return ToColor(LinearColor(p, value, t));
            case 3: return ToColor(LinearColor(p, q, value));
            case 4: return ToColor(LinearColor(t, p, value));
            case 5: return ToColor(LinearColor(value, p, q));

            // Error
            default: return Color(0, 0, 0);
        }
    }
}