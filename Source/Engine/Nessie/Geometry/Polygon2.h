// Polygon2.h
#pragma once
#include "Segment.h"

namespace nes
{
    class Polygon2
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Iterator that traverses the edges of the polygon. 
        //----------------------------------------------------------------------------------------------------
        class SegmentIterator
        {
            friend class Polygon2;
            SegmentIterator(const Polygon2* pPoly, const size_t currentIndex) : m_pPoly(pPoly), m_index(currentIndex) {}

        public:
            inline Segment2         operator*() const;
            inline SegmentIterator& operator++();
            inline SegmentIterator  operator++(int);
            bool                    operator==(const SegmentIterator& other) const { return m_index == other.m_index && m_pPoly == other.m_pPoly; }
            bool                    operator!=(const SegmentIterator& other) const { return !(*this == other); }

        private:
            const Polygon2*         m_pPoly = nullptr;
            size_t                  m_index = 0;
        };

    public:
        /// Constructors
        Polygon2() = default;
        Polygon2(const std::vector<Vec2>& vertices) : m_vertices(vertices) {}

        /// Index Operators
        const Vec2&                 operator[](const size_t index) const    { NES_ASSERT(index < m_vertices.size()); return m_vertices[index]; }
        Vec2&                       operator[](const size_t index)          { NES_ASSERT(index < m_vertices.size()); return m_vertices[index]; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns an iterator to the first segment of the polygon. 
        //----------------------------------------------------------------------------------------------------
        SegmentIterator             begin() const       { return SegmentIterator(this, 0); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns an iterator past the last segment. 
        //----------------------------------------------------------------------------------------------------
        SegmentIterator             end() const         { return SegmentIterator(this, m_vertices.size()); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the vertices that make up this polygon. 
        //----------------------------------------------------------------------------------------------------
        const std::vector<Vec2>&    GetVertices() const { return m_vertices; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of vertices that make up this polygon. 
        //----------------------------------------------------------------------------------------------------
        size_t                      NumVertices() const { return m_vertices.size(); }

    private:
        std::vector<Vec2>           m_vertices{};
    };
}

namespace nes
{
    inline Segment2 Polygon2::SegmentIterator::operator*() const
    {
        // Return the segment from the current to the next (looping back to the start).
        NES_ASSERT(m_pPoly != nullptr && m_index < m_pPoly->m_vertices.size());
        return Segment2(m_pPoly->m_vertices[m_index], m_pPoly->m_vertices[(m_index + 1) % m_pPoly->m_vertices.size()]);
    }
    
    inline Polygon2::SegmentIterator& Polygon2::SegmentIterator::operator++()
    {
        ++m_index;
        return *this;
    }
    
    inline Polygon2::SegmentIterator Polygon2::SegmentIterator::operator++(int)
    {
        const SegmentIterator tmp = *this;
        ++m_index;
        return tmp;
    }
}
