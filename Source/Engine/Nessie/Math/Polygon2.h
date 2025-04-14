// Polygon2.h
#pragma once
#include "Segment.h"
#include "Vector2.h"

namespace nes
{
    template <FloatingPointType Type>
    class TPolygon2
    {
        std::vector<TVector2<Type>> m_vertices;

    public:
        class SegmentIterator
        {
            friend class TPolygon2;
            TPolygon2* m_pPoly = nullptr;
            size_t m_index = 0;

            SegmentIterator(TPolygon2* pPoly, const size_t currentIndex) : m_pPoly(pPoly), m_index(currentIndex) {}

        public:
            TSegment2<Type> operator*() const;
            SegmentIterator& operator++();
            SegmentIterator& operator++(int);
            bool operator==(const SegmentIterator& other) const;
            bool operator!=(const SegmentIterator& other) const { return !(*this == other); }
        };

    public:
        TPolygon2() = default;
        TPolygon2(const std::vector<TVector2<Type>>& vertices);

        const TVector2<Type>& operator[](const size_t index) const;
        TVector2<Type>& operator[](const size_t index);

        SegmentIterator begin() const;
        SegmentIterator end() const;
        const std::vector<TVector2<Type>>& GetVertices() const { return m_vertices; }
        size_t NumVertices() const { return m_vertices.size(); }
    };

    using Polygon2f = TPolygon2<float>;
    using Polygon2d = TPolygon2<double>;
    using Polygon2D = TPolygon2<NES_PRECISION_TYPE>;
}

namespace nes
{
    template <FloatingPointType Type>
    TSegment2<Type> TPolygon2<Type>::SegmentIterator::operator*() const
    {
        // Return the segment from the current to the next (looping back to the start).
        NES_ASSERT(m_pPoly != nullptr && m_index < m_pPoly->m_vertices.size());
        return TSegment2<Type>(m_vertices[m_index], m_vertices[(m_index + 1) % m_vertices.size()]);
    }

    template <FloatingPointType Type>
    typename TPolygon2<Type>::SegmentIterator& TPolygon2<Type>::SegmentIterator::operator++()
    {
        ++m_index;
        return *this;
    }
    
    template <FloatingPointType Type>
    typename TPolygon2<Type>::SegmentIterator& TPolygon2<Type>::SegmentIterator::operator++(int)
    {
        SegmentIterator tmp = *this;
        ++m_index;
        return tmp;
    }

    template <FloatingPointType Type>
    bool TPolygon2<Type>::SegmentIterator::operator==(const SegmentIterator& other) const
    {
        return m_index == other.m_index && m_pPoly == other.m_pPoly;
    }

    template <FloatingPointType Type>
    TPolygon2<Type>::TPolygon2(const std::vector<TVector2<Type>>& vertices)
        : m_vertices(vertices)
    {
        //
    }

    template <FloatingPointType Type>
    const TVector2<Type>& TPolygon2<Type>::operator[](const size_t index) const
    {
        NES_ASSERT(index < m_vertices.size());
        return m_vertices[index];
    }

    template <FloatingPointType Type>
    TVector2<Type>& TPolygon2<Type>::operator[](const size_t index)
    {
        NES_ASSERT(index < m_vertices.size());
        return m_vertices[index];
    }

    template <FloatingPointType Type>
    typename TPolygon2<Type>::SegmentIterator TPolygon2<Type>::begin() const
    {
        return SegmentIterator(*this, 0);
    }

    template <FloatingPointType Type>
    typename TPolygon2<Type>::SegmentIterator TPolygon2<Type>::end() const
    {
        return SegmentIterator(*this, m_vertices.size());
    }
}
