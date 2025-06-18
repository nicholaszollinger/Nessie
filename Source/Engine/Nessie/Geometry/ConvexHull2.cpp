// ConvexHull2.cpp
#include "ConvexHull2.h"

namespace nes
{
    bool ConvexHull2::TrySolve(const std::vector<Vec2>& points)
    {
        // This uses Andrew's Monotone chain Algorithm. This works well in 2D, but QuickHull will need to
        //  be implemented in 3D.
        //	Wikipedia: https://en.wikibooks.org/wiki/Algorithm_Implementation/Geometry/Convex_hull/Monotone_chain
        
        m_dimension = 0;
        m_hullIndices.clear();
        m_hullIndices.reserve(points.size());
        
        // Sort the points:
        for (size_t i = 0; i < points.size(); ++i)
        {
            m_hullIndices.emplace_back(i);
        }

        nes::QuickSort(m_hullIndices.begin(), m_hullIndices.end(), [&points](const size_t a, const size_t b)
        {
            // Sort by x, break ties with y.
            return (points[a][0] < points[b][0]) || (points[a][0] == points[b][0] && points[a][1] < points[b][1]);
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
        Vec2 firstSegment[2] = { points[m_hullIndices[0]], points[m_hullIndices[1]] };
        bool foundSecondDimension = false;
        
        for (size_t i = 2; i < m_hullIndices.size(); ++i)
        {
            Vec2 next = points[m_hullIndices[i]];
            if (!math::PointsAreCollinear(firstSegment[0], firstSegment[1], next))
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
    
    void ConvexHull2::Clear()
    {
        m_hullIndices.clear();
        m_dimension = 0;
    }
}