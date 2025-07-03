// Rect.h
#pragma once
#include "Nessie/Math/Vec2.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //  Might rename this to a Quad? This was previously used in collision, but that was changed to
    //  "AABox".
    //  
    ///	@brief : Represents a quad for rendering purposes. The Position is the bottom-left corner of the Rect.
    ///	@tparam Type : Type of the Rect.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type>
    struct Rect
    {
        static_assert(false, "Do not use this class! It's broken!");
        
        Type x;
        Type y;
        Type width;
        Type height;

        constexpr Rect() = default;
        constexpr Rect(const Type x, const Type y, const Type width, const Type height);
        constexpr Rect(const TVector2<Type> position, const TVector2<Type> size);

        /// Operators
        constexpr bool              operator==(const Rect& other) const;
        constexpr bool              operator!=(const Rect& other) const { return !(*this == other); }
        constexpr Rect&             operator+=(const TVector2<Type> point);
        constexpr Rect&             operator+=(const Rect& other);

        void                        SetPosition(const Type _x, const Type _y);
        void                        SetPosition(const TVector2<Type> pos);
        void                        SetSize(const Type _width, const Type _height);
        void                        SetSize(const TVector2<Type> size);

        constexpr TVector2<Type>    GetPosition() const;
        constexpr TVector2<Type>    GetSize() const;
        constexpr TVector2<Type>    Center() const;
        constexpr TVector2<Type>    Min() const;
        constexpr TVector2<Type>    Max() const;

        constexpr bool              HasValidDimensions() const;
        TVector2<Type>              Extent() const;

        std::string                 ToString() const;
    };

    using Rectf = Rect<float>;
    using Rectd = Rect<double>;
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
    constexpr Rect<Type>::Rect(const TVector2<Type> position, const TVector2<Type> size)
        : x(position.x)
        , y(position.y)
        , width(size.x)
        , height(size.y)
    {
        //
    }

    template <ScalarType Type>
    constexpr bool Rect<Type>::operator==(const Rect& other) const
    {
        return x == other.x && y == other.y && width == other.width && height == other.height;
    }

    template <ScalarType Type>
    constexpr Rect<Type>& Rect<Type>::operator+=(const TVector2<Type> point)
    {
        if (HasValidDimensions())
        {
            x = math::Min(x, point.x);
            y = math::Min(y, point.y);
            width = math::Max(width, math::Abs<Type>(point.x - x));
            height = math::Max(height, math::Abs<Type>(point.y - y));
        }

        else
        {
            // Hacky way to make sure the rect is valid for the first point.
            // Unreal handles this by storing the Rect as a Min and Max point,
            // along with a bool that is set to true when the Min and Max are set.
            x = point.x;
            y = point.y;
            width = static_cast<Type>(math::PrecisionDelta());
            height = static_cast<Type>(math::PrecisionDelta());
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
            const Type minX = math::Min(x, other.x);
            const Type minY = math::Min(y, other.y);
            const Type maxX = math::Max(x + width, other.x + other.width);
            const Type maxY = math::Max(y + height, other.y + other.height);

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
    void Rect<Type>::SetPosition(const TVector2<Type> pos)
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
    void Rect<Type>::SetSize(const TVector2<Type> size)
    {
        width = size.x;
        height = size.y;
    }

    template <ScalarType Type>
    constexpr TVector2<Type> Rect<Type>::GetPosition() const
    {
        return TVector2<Type>(x, y);
    }

    template <ScalarType Type>
    constexpr TVector2<Type> Rect<Type>::GetSize() const
    {
        return TVector2<Type>(width, height);
    }

    template <ScalarType Type>
    constexpr TVector2<Type> Rect<Type>::Center() const
    {
        return TVector2<Type>(x + width / 2, y + height / 2);
    }

    template <ScalarType Type>
    constexpr TVector2<Type> Rect<Type>::Min() const
    {
        return TVector2<Type>(x, y);
    }

    template <ScalarType Type>
    constexpr TVector2<Type> Rect<Type>::Max() const
    {
        return TVector2<Type>(x + width, y + height);
    }

    template <ScalarType Type>
    constexpr bool Rect<Type>::HasValidDimensions() const
    {
        return width > static_cast<Type>(0) && height > static_cast<Type>(0);
    }

    template <ScalarType Type>
    TVector2<Type> Rect<Type>::Extent() const
    {
        return GetSize() / static_cast<Type>(2); 
    }

    template <ScalarType Type>
    std::string Rect<Type>::ToString() const
    {
        return CombineIntoString("(x=", x, ", y=" , y, ", width=", width, ", height=", height, ")");
    }
}
