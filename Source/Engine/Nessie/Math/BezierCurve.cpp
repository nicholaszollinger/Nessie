// BezierCurve.cpp

#include "BezierCurve.h"

namespace nes
{
    BezierCurve::BezierCurve()
        : m_start(0.0f, 0.0f)
        , m_end(1.0f, 1.0f)
        , m_control1(0.5f, 0.0f)
        , m_control2(0.5f, 1.0f)
    {
        //
    }

    BezierCurve::BezierCurve(const float start, const float end, const Vec2 control1, const Vec2 control2)
        : m_start(0.0f, start)
        , m_end(1.0f, end)
        , m_control1(control1)
        , m_control2(control2)
    {
        //
    }

    float BezierCurve::Evaluate(const float t) const
    {
        const float t2 = t * t;
        const float t3 = t2 * t;

        // a = (1 - t)^3
        const float a = -t3 + 3 * t2 - 3 * t + 1;

        // b = 3t(1 - t)^2
        const float b = 3 * t * (t2 - 2 * t + 1);

        // c = 3t^2(1 - t)
        const float c = 3 * t2 * (1 - t);

        // d = t^3 = t3
        const Vec2 result = m_start * a + m_control1 * b + m_control2 * c + m_end * t3;
        return result.y;
    }
}