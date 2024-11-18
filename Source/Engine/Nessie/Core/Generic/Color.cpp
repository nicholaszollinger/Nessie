// Color.cpp
#include "Color.h"

namespace nes
{
    constexpr Color::Color(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
        : r(r)
        , g(g)
        , b(b)
        , a(a)
    {
        //
    }

    constexpr bool Color::operator==(const Color& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    constexpr LinearColor::LinearColor(const float r, const float g, const float b, const float a)
        : r(r)
        , g(g)
        , b(b)
        , a(a)
    {
        //
    }

    constexpr Color ToColor(const LinearColor& linearColor)
    {
        return Color(
            static_cast<uint8_t>(linearColor.r * 255.0f),
            static_cast<uint8_t>(linearColor.g * 255.0f),
            static_cast<uint8_t>(linearColor.b * 255.0f),
            static_cast<uint8_t>(linearColor.a * 255.0f)
        );
    }

    constexpr LinearColor ToLinearColor(const Color& color)
    {
        return LinearColor(
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