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
        Vector2f m_start;
        Vector2f m_end;
        Vector2f m_control1;
        Vector2f m_control2;

    public:
        BezierCurve();
        BezierCurve(const float start, const float end, const Vector2f control1, const Vector2f control2);
        float Evaluate(const float t) const;
    };
}
