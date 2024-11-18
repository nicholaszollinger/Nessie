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

        void SetPosition(const Type _x, const Type _y);
        void SetPosition(const Vector2<Type> pos);
        void SetSize(const Type _width, const Type _height);
        void SetSize(const Vector2<Type> size);

        constexpr Vector2<Type> GetPosition() const;
        constexpr Vector2<Type> GetSize() const;
        constexpr Vector2<Type> GetCenter() const;

        constexpr Type Left() const;
        constexpr Type Right() const;
        constexpr Type Top() const;
        constexpr Type Bottom() const;

        constexpr bool HasValidDimensions() const;
        constexpr bool Intersects(const Rect other) const;
        constexpr bool Contains(const Vector2<Type> point) const;
        constexpr bool Contains(const Rect other) const;
        constexpr Rect GetIntersectionAsRect(const Rect<Type> other) const;

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
    constexpr Type Rect<Type>::Left() const
    {
        return x;
    }

    template <ScalarType Type>
    constexpr Type Rect<Type>::Right() const
    {
        return x + width;
    }

    template <ScalarType Type>
    constexpr Type Rect<Type>::Top() const
    {
        return y + height;
    }

    template <ScalarType Type>
    constexpr Type Rect<Type>::Bottom() const
    {
        return y;
    }

    template <ScalarType Type>
    constexpr bool Rect<Type>::HasValidDimensions() const
    {
        return width > static_cast<Type>(0) && height > static_cast<Type>(0);
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
    constexpr Rect<Type> Rect<Type>::GetIntersectionAsRect(const Rect other) const
    {
        Rect result;
        result.x = std::max(x, other.x);
        result.y = std::max(y, other.y);
        result.width = std::min(x + width, other.x + other.width) - other.x;
        result.height = std::min(y + height, other.y + other.height) - other.y;

        return result;
    }

    template <ScalarType Type>
    std::string Rect<Type>::ToString() const
    {
        return CombineIntoString("(x=", x, ", y=" , y, ", width=", width, ", height=", height, ")");
    }
}
