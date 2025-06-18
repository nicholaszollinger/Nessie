// GJKClosestPoint.cpp
#include "GJKClosestPoint.h"

#include "ClosestPoint.h"
#include "Math/Bit.h"

namespace nes
{
    float GJKClosestPoint::GetMaxYLengthSqr() const
    {
        float yLengthSqr = m_y[0].LengthSqr();
        for (int i = 1; i < m_numPoints; ++i)
            yLengthSqr = math::Max(yLengthSqr, m_y[i].LengthSqr());

        return yLengthSqr;
    }

    void GJKClosestPoint::UpdatePointSetY(const uint32_t set)
    {
        int numPoints = 0;
        for (int i = 0; i < m_numPoints; ++i)
        {
            if (math::IsBitSet(set, i))
            {
                m_y[numPoints] = m_y[i];
                ++numPoints;
            }
        }

        m_numPoints = numPoints;
    }

    void GJKClosestPoint::UpdatePointSetP(const uint32_t set)
    {
        int numPoints = 0;
        for (int i = 0; i < m_numPoints; ++i)
        {
            if (math::IsBitSet(set, i))
            {
                m_p[numPoints] = m_p[i];
                ++numPoints;
            }
        }

        m_numPoints = numPoints;
    }

    void GJKClosestPoint::UpdatePointSetPQ(const uint32_t set)
    {
        int numPoints = 0;
        for (int i = 0; i < m_numPoints; ++i)
        {
            if (math::IsBitSet(set, i))
            {
                m_p[numPoints] = m_p[i];
                m_q[numPoints] = m_q[i];
                ++numPoints;
            }
        }

        m_numPoints = numPoints;
    }

    void GJKClosestPoint::UpdatePointSetYPQ(const uint32_t set)
    {
        int numPoints = 0;
        for (int i = 0; i < m_numPoints; ++i)
        {
            if (math::IsBitSet(set, i))
            {
                m_y[numPoints] = m_y[i];
                m_p[numPoints] = m_p[i];
                m_q[numPoints] = m_q[i];
                ++numPoints;
            }
        }

        m_numPoints = numPoints;
    }

    void GJKClosestPoint::CalculatePointAAndB(Vec3& outPointA, Vec3& outPointB) const
    {
        switch (m_numPoints)
        {
            case 1:
            {
                outPointA = m_p[0];
                outPointB = m_q[0];
                break;
            }

            case 2:
            {
                float u, v;
                ClosestPoint::GetBaryCentricCoordinates(m_y[0], m_y[1], u, v);
                outPointA = (u * m_p[0]) + (v * m_p[1]);
                outPointB = (u * m_q[0]) + (v * m_q[1]);
                break;
            }

            case 3:
            {
                float u, v, w;
                ClosestPoint::GetBaryCentricCoordinates(m_y[0], m_y[1], m_y[2], u, v, w);
                outPointA = u * m_p[0] + v * m_p[1] + w * m_p[2];
                outPointB = u * m_q[0] + v * m_q[1] + w * m_q[2];
                break;
            }

            case 4:
            {
            #ifdef NES_DEBUG
                memset(&outPointA, 0xcd, sizeof(outPointA));    
                memset(&outPointB, 0xcd, sizeof(outPointB));    
            #endif
                break;
            }
            
            default:
            {
                NES_ASSERT(false);
                break;
            }
        }
    }

    void GJKClosestPoint::GetClosestPointsSimplex(Vec3* pOutY, Vec3* pOutP, Vec3* pOutQ, size_t& outNumPoints) const
    {
        unsigned size = sizeof(Vec3) * m_numPoints;
        memcpy(pOutY, m_y, size);
        memcpy(pOutP, m_p, size);
        memcpy(pOutQ, m_q, size);
        outNumPoints = m_numPoints;
    }
}
