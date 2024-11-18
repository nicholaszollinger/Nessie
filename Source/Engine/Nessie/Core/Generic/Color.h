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
        constexpr Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a = 255);

        constexpr bool operator==(const Color& other) const;
        constexpr bool operator!=(const Color& other) const { return !(*this == other); }

        static constexpr Color White()  { return {}; }
        static constexpr Color Black()  { return {0, 0, 0 }; }
        static constexpr Color Red()    { return { 255, 0, 0 }; }
        static constexpr Color Green()  { return { 0, 255, 0 }; }
        static constexpr Color Blue()   { return { 0, 0, 255 }; }
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
        constexpr LinearColor(const float r, const float g, const float b, const float a = 1.0f);

        //constexpr bool operator==(const LinearColor& other) const;
        //constexpr bool operator!=(const LinearColor& other) const { return !(*this == other); }

        static constexpr LinearColor White()    { return {}; }
        static constexpr LinearColor Black()    { return { 0.0f, 0.0f, 0.0f }; }
        static constexpr LinearColor Red()      { return { 1.0f, 0.0f, 0.0f }; }
        static constexpr LinearColor Green()    { return { 0.0f, 1.0f, 0.0f }; }
        static constexpr LinearColor Blue()     { return { 0.0f, 0.0f, 1.0f }; }
    };

    constexpr Color ToColor(const LinearColor& linearColor);
    constexpr LinearColor ToLinearColor(const Color& color);
    constexpr Color HSVtoRGB(const float hue, const float saturation, const float value);
}