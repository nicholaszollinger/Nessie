#pragma once
// BezierCurve.h
#include "Vector2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //      NOTES:
    //      Nick: Use this Desmos Graph for tuning: https://www.desmos.com/calculator/safvsnwhjz
    //     
    ///	    @brief :  Bezier Curve object. Default constructor creates a S-Curve from 0 to 1.
    //----------------------------------------------------------------------------------------------------
    class BezierCurve
    {
        Vec2 m_start;
        Vec2 m_end;
        Vec2 m_control1;
        Vec2 m_control2;

    public:
        BezierCurve();
        BezierCurve(const float start, const float end, const Vec2 control1, const Vec2 control2);
        float Evaluate(const float t) const;
    };
}
