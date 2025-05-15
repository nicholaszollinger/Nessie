// ConvexHull2.h
#pragma once
#include <algorithm>
#include "Math/Geometry.h"
#include "Math/Vector2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : A Convex Hull is a convex bounding polygon around a set of points. This class stores
    ///             the indices of the passed in set of points that make up the bounding polygon. It is meant
    ///             to be used in tandem with the set of points it is made from.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    class TConvexHull2
    {
        std::vector<size_t> m_hullIndices{};
        int m_dimension = 0;
        
    public:
        TConvexHull2() = default;

        bool TrySolve(const std::vector<TVector2<Type>>& points);
        void Clear();
        
        [[nodiscard]] bool IsValid() const;
        [[nodiscard]] int GetDimension() const { return m_dimension; }
        [[nodiscard]] const std::vector<size_t>& GetHullIndices() const { return m_hullIndices; }
    };

    using ConvexHull2f = TConvexHull2<float>;
    using ConvexHull2d = TConvexHull2<double>;
    using ConvexHull2D = TConvexHull2<NES_PRECISION_TYPE>;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      This uses Andrew's Monotone chain Algorithm. This works well in 2D, but QuickHull will need to
    //      be implemented in 3D.
    //		Wikipedia: https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
    ///		@brief : Attempt to create a Convex Hull from the set of points.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TConvexHull2<Type>::TrySolve(const std::vector<TVector2<Type>>& points)
    {
        m_dimension = 0;
        m_hullIndices.clear();
        m_hullIndices.reserve(points.size());
        
        // Sort the points:
        for (size_t i = 0; i < points.size(); ++i)
        {
            m_hullIndices.emplace_back(i);
        }

        std::sort(m_hullIndices.begin(), m_hullIndices.end(), [&points](const size_t a, const size_t b)
        {
            // Sort by x, break ties with y.
            return (points[a].x < points[b].x) || (points[a].x == points[b].x && points[a].y < points[b].y);
        });

        // Ensure that the vertices are unique.
        const auto result = std::unique(m_hullIndices.begin(), m_hullIndices.end(), [&points](const size_t a, const size_t b)
        {
            return points[a] == points[b]; 
        });
        m_hullIndices.erase(result, m_hullIndices.end());

        // Degenerate Result, resulting in either nothing, a point, or a 2D line.
        if (m_hullIndices.size() < 3)
        {
            m_dimension = static_cast<int>(math::Max<size_t>(0, m_hullIndices.size() - 1)); 
            return false;
        }
        
        // Check for collinearity:
        TVector2<Type> firstSegment[2] = { points[m_hullIndices[0]], points[m_hullIndices[1]] };
        bool foundSecondDimension = false;
        
        for (size_t i = 2; i < m_hullIndices.size(); ++i)
        {
            TVector2<Type> next = points[m_hullIndices[i]];
            if (!math::PointsAreCollinear<Type>(firstSegment[0], firstSegment[1], next))
            {
                foundSecondDimension = true;
                break;
            }
        }
        
        // All points are collinear
        if (!foundSecondDimension)
        {
            m_dimension = 1;
            return false;
        }
        
        m_dimension = 2;

        std::vector<size_t> chainIndices(2 * m_hullIndices.size());
        size_t k = 0; // number of actual hull indices

        // Build the Lower Hull:
        for (size_t i = 0; i < m_hullIndices.size(); ++i)
        {
            while (k >= 2 && math::Orient2D(points[chainIndices[k - 2]], points[chainIndices[k - 1]], points[m_hullIndices[i]]) < 0.f)
            {
                --k;
            }

            chainIndices[k] = m_hullIndices[i];
            ++k;
        }

        // Build the Upper Hull:
        for (size_t i = m_hullIndices.size() - 1, t = k + 1; i > 0; --i)
        {
            while (k >= t && math::Orient2D(points[chainIndices[k - 2]], points[chainIndices[k - 1]], points[m_hullIndices[i - 1]]) < 0.f)
            {
                --k;
            }

            chainIndices[k] = m_hullIndices[i - 1];
            ++k;
        }

        chainIndices.resize(k - 1);
        m_hullIndices.swap(chainIndices);
        
        return true;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Clears the previously solved solution for a set of points. 
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    void TConvexHull2<Type>::Clear()
    {
        m_hullIndices.clear();
        m_dimension = 0;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the dimension of the solved Hull is equal to 2. If you haven't called
    ///             TrySolve(), this is guaranteed to be false.
    //----------------------------------------------------------------------------------------------------
    template <FloatingPointType Type>
    bool TConvexHull2<Type>::IsValid() const
    {
        return m_dimension == 2;
    }
}