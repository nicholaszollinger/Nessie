#pragma once
// Rect.h
#include "Vector2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : 2D Rect class. The Position is the bottom-left corner of the Rect.
    ///		@tparam Type : Type of the Rect.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    struct Rect
    {
        Type x;
        Type y;
        Type width;
        Type height;

        constexpr Rect() = default;
        constexpr Rect(const Type x, const Type y, const Type width, const Type height);
        constexpr Rect(const Vector2<Type> position, const Vector2<Type> size);
        constexpr Rect(const std::vector<Vector2<Type>>& points);
        constexpr Rect(Vector2<Type>* pPoints, const size_t count);

        constexpr bool operator==(const Rect& other) const;
        constexpr bool operator!=(const Rect& other) const { return !(*this == other); }
        constexpr Rect& operator+=(const Vector2<Type> point);
        constexpr Rect& operator+=(const Rect& other);

        void SetPosition(const Type _x, const Type _y);
        void SetPosition(const Vector2<Type> pos);
        void SetSize(const Type _width, const Type _height);
        void SetSize(const Vector2<Type> size);

        constexpr Vector2<Type> GetPosition() const;
        constexpr Vector2<Type> GetSize() const;
        constexpr Vector2<Type> GetCenter() const;
        constexpr Vector2<Type> Min() const;
        constexpr Vector2<Type> Max() const;

        constexpr bool HasValidDimensions() const;
        constexpr Type GetArea() const;
        constexpr bool Intersects(const Rect other) const;
        constexpr bool Contains(const Vector2<Type> point) const;
        constexpr bool Contains(const Rect other) const;
        constexpr Rect GetOverlap(const Rect<Type> other) const;
        constexpr Type ComputeSquaredDistanceToPoint(const Vector2<Type> point) const;
        Vector2<Type> GetClosestPontTo(const Vector2<Type> point) const;
        Vector2<Type> GetExtent() const;

        std::string ToString() const;
    };

    using Rectf = Rect<float>;
    using Recti = Rect<int>;
    using Rectu = Rect<unsigned int>;
}

namespace nes
{
    template <ScalarType Type>
    constexpr Rect<Type>::Rect(const Type x, const Type y, const Type width, const Type height)
        : x(x)
        , y(y)
        , width(width)
        , height(height)
    {
        //
    }

    template <ScalarType Type>
    constexpr Rect<Type>::Rect(const Vector2<Type> position, const Vector2<Type> size)
        : x(position.x)
        , y(position.y)
        , width(size.x)
        , height(size.y)
    {
        //
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Constructs a bounding Rect containing all the points in the vector.
    ///		@param points : Points to include in the bounding Rect.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Rect<Type>::Rect(const std::vector<Vector2<Type>>& points)
        : Rect()
    {
        for (const auto point : points)
        {
            *this += point;
        }
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Constructs a bounding Rect containing all the points in the array.
    ///		@param pPoints : Array of points.
    ///		@param count : Number of points in the array.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Rect<Type>::Rect(Vector2<Type>* pPoints, const size_t count)
        : Rect()
    {
        NES_ASSERTV(pPoints != nullptr, "Invalid points array!");
        for (size_t i = 0; i < count; ++i)
        {
            *this += pPoints[i];
        }
    }

    template <ScalarType Type>
    constexpr bool Rect<Type>::operator==(const Rect& other) const
    {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    template <ScalarType Type>
    constexpr Rect<Type>& Rect<Type>::operator+=(const Vector2<Type> point)
    {
        if (HasValidDimensions())
        {
            x = nmath::Min(x, point.x);
            y = nmath::Min(y, point.y);
            width = nmath::Max(width, point.x - x);
            height = nmath::Max(height, point.y - y);
        }

        else
        {
            // Hacky way to make sure the rect is valid for the first point.
            // Unreal handles this by storing the Rect as a Min and Max point,
            // along with a bool that is set to true when the Min and Max are set.
            x = point.x;
            y = point.y;
            width = static_cast<Type>(nmath::PrecisionDelta());
            height = static_cast<Type>(nmath::PrecisionDelta());
        }

        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Increases the size of the Rect to include the other Rect.
    ///		@param other : Other rect to include.
    ///		@returns : Reference to this Rect.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Rect<Type>& Rect<Type>::operator+=(const Rect& other)
    {
        if (HasValidDimensions() && other.HasValidDimensions())
        {
            const Type minX = nmath::Min(x, other.x);
            const Type minY = nmath::Min(y, other.y);
            const Type maxX = nmath::Max(x + width, other.x + other.width);
            const Type maxY = nmath::Max(y + height, other.y + other.height);

            x = minX;
            y = minY;
            width = maxX - minX;
            height = maxY - minY;
        }

        else if (other.HasValidDimensions())
        {
            *this = other;
        }

        return *this;
    }



    template <ScalarType Type>
    void Rect<Type>::SetPosition(const Type _x, const Type _y)
    {
        x = _x;
        y = _y;
    }

    template <ScalarType Type>
    void Rect<Type>::SetPosition(const Vector2<Type> pos)
    {
        x = pos.x;
        y = pos.y;
    }

    template <ScalarType Type>
    void Rect<Type>::SetSize(const Type _width, const Type _height)
    {
        width = _width;
        height = _height;
    }

    template <ScalarType Type>
    void Rect<Type>::SetSize(const Vector2<Type> size)
    {
        width = size.x;
        height = size.y;
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Rect<Type>::GetPosition() const
    {
        return Vector2<Type>(x, y);
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Rect<Type>::GetSize() const
    {
        return Vector2<Type>(width, height);
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Rect<Type>::GetCenter() const
    {
        return Vector2<Type>(x + width / 2, y + height / 2);
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Rect<Type>::Min() const
    {
        return Vector2<Type>(x, y);
    }

    template <ScalarType Type>
    constexpr Vector2<Type> Rect<Type>::Max() const
    {
        return Vector2<Type>(x + width, y + height);
    }

    template <ScalarType Type>
    constexpr bool Rect<Type>::HasValidDimensions() const
    {
        return width > static_cast<Type>(0) && height > static_cast<Type>(0);
    }

    template <ScalarType Type>
    constexpr Type Rect<Type>::GetArea() const
    {
        return width * height;
    }

    template <ScalarType Type>
    constexpr bool Rect<Type>::Intersects(const Rect other) const
    {
        return x < other.x + other.width
            && x + width > other.x
            && y < other.y + other.height
            && y + height > other.y;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the point is inside this Rect.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool Rect<Type>::Contains(const Vector2<Type> point) const
    {
        return point.x >= x
            && point.x <= x + width
            && point.y >= y
            && point.y <= y + height;
    }

    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns true if the other Rect is inside this Rect.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr bool Rect<Type>::Contains(const Rect other) const
    {
        return x <= other.x
            && x + width >= other.x + other.width
            && y <= other.y
            && y + height >= other.y + other.height;
    }

    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Calculates the intersection of this and another Rect and returns the overlapping area rect. If there is
    ///             no intersection, then the width and height would be <= 0, meaning it's invalid. Use
    ///             HasValidDimensions() on the resulting Rect to check if the intersection is valid.
    //-----------------------------------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Rect<Type> Rect<Type>::GetOverlap(const Rect other) const
    {
        Rect result;
        result.x = nmath::Max(x, other.x);
        result.y = nmath::Max(y, other.y);
        result.width = nmath::Min(x + width, other.x + other.width) - other.x;
        result.height = nmath::Min(y + height, other.y + other.height) - other.y;

        return result;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Calculates the Distance Squared of a point to this Rect. If the point is inside, the
    ///              value would be 0.
    ///		@param point : The point to test.
    ///		@returns : Squared Distance.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    constexpr Type Rect<Type>::ComputeSquaredDistanceToPoint(const Vector2<Type> point) const
    {
        Type distSquared = 0.f;

        if (point.x < x)
            distSquared += nmath::Squared(x - point.x);

        else if (point.x > x + width)
            distSquared += nmath::Squared(point.x - (x + width));

        else if (point.y < y)
            distSquared += nmath::Squared(y - point.y);

        else if (point.y > y + height)
            distSquared += nmath::Squared(point.y - (y + height));

        return distSquared;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Get the closest point on the Rect to the given point. If the point is inside the
    ///              rect, then the point itself is returned.
    ///		@param point : Reference point.
    ///		@returns : Closest point on the rect.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    Vector2<Type> Rect<Type>::GetClosestPontTo(const Vector2<Type> point) const
    {
        Vector2<Type> closestPoint = point;

        // Clamp to X Dimension
        if (point.x < x)
            closestPoint.x = x;

        else if (point.x > x + width)
            closestPoint.x = x + width;

        // Then clamp to Y Dimension
        if (point.y < y)
            closestPoint.y = y;

        else if (point.y > y + height)
            closestPoint.y = y + height;

        return closestPoint;
    }

    template <ScalarType Type>
    Vector2<Type> Rect<Type>::GetExtent() const
    {
        return GetSize() / static_cast<Type>(2); 
    }

    template <ScalarType Type>
    std::string Rect<Type>::ToString() const
    {
        return CombineIntoString("(x=", x, ", y=" , y, ", width=", width, ", height=", height, ")");
    }
}
