// ConvexHull2.h
#pragma once
#include <algorithm>
#include "Geometry.h"
#include "Core/QuickSort.h"
#include "Math/Vec2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : A Convex Hull is a convex bounding polygon around a set of points. This class stores
    ///   the indices of the passed in set of points that make up the bounding polygon. It is meant
    ///   to be used in tandem with the set of points it is made from.
    //----------------------------------------------------------------------------------------------------
    class ConvexHull2
    {
    public:
        ConvexHull2() = default;

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Attempt to create a Convex Hull from the set of points.
        //----------------------------------------------------------------------------------------------------
        bool                        TrySolve(const std::vector<Vec2>& points);

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Clears the previously solved solution for a set of points. 
        //----------------------------------------------------------------------------------------------------
        void                        Clear();

        //----------------------------------------------------------------------------------------------------
        ///	@brief : Returns true if the dimension of the solved Hull is equal to 2. If you haven't called
        ///     TrySolve(), this is guaranteed to be false.
        //----------------------------------------------------------------------------------------------------
        [[nodiscard]] bool          IsValid() const             { return m_dimension == 2; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the resulting dimension after calling TrySolve(). 
        //----------------------------------------------------------------------------------------------------
        int                         GetDimension() const        { return m_dimension; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the resulting array of indices after calling TrySolve(). 
        //----------------------------------------------------------------------------------------------------
        const std::vector<size_t>&  GetHullIndices() const      { return m_hullIndices; }

    private:
        std::vector<size_t>         m_hullIndices{}; /// Indices of the points array parameter in TrySolve().
        int                         m_dimension = 0; /// Resulting dimension after TrySolve().
    };
}