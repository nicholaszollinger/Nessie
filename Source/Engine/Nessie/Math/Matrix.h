// Matrix.h
#pragma once
#include "Input/InputCodes.h"
#include "Math/Generic.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Returns the difference of (a1 * a2) and (b1 * b2). 
    ///		@tparam Type : Scalar Type of the values.
    //----------------------------------------------------------------------------------------------------
    template <ScalarType Type = float>
    constexpr Type DifferenceOfProducts(const Type a1, const Type a2, const Type b1, const Type b2)
    {
        return a1 * a2 - b1 * b2;
    }
    
    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      I am only really interested in supporting 2x2, 3x3 & 4x4 matrices, for making transformations.
    //      - Next I need to make Multiplication operators functions for different sized vectors.
    //          - So I need to make a Vec3 and Vec4.
    //		
    ///		@brief : A Square Matrix.
    ///		@tparam N : Dimension of the Square Matrix. Ex: N == 2 would be a 2x2 Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <int N, FloatingPointType Type>
    struct SquareMatrix
    {
        Type m[N][N] {};
        
        constexpr SquareMatrix();
        constexpr SquareMatrix(const Type values[N * N]);
        constexpr SquareMatrix(const SquareMatrix&) = default;
        constexpr SquareMatrix(SquareMatrix&&) = default;
        constexpr SquareMatrix& operator=(const SquareMatrix&) = default;
        constexpr SquareMatrix& operator=(SquareMatrix&&) = default;
        
        constexpr bool operator==(const SquareMatrix& other) const;
        constexpr bool operator!=(const SquareMatrix& other) const;
        SquareMatrix  operator+(const SquareMatrix& other) const;
        SquareMatrix& operator+=(const SquareMatrix& other);
        SquareMatrix  operator-(const SquareMatrix& other) const;
        SquareMatrix& operator-=(const SquareMatrix& other);
        SquareMatrix  operator*(const SquareMatrix& other) const;
        SquareMatrix& operator*=(const SquareMatrix& other);
        SquareMatrix  operator*(const float scalar);
        SquareMatrix& operator*=(const float scalar);
        
        bool TryInvert();
        bool TryGetInverse(SquareMatrix& result) const;
        SquareMatrix& Transpose();
        SquareMatrix GetTranspose() const;
        float CalculateDeterminant() const;
        bool IsIdentity() const;
        
        std::string ToString() const;
        
        static constexpr SquareMatrix Zero();
        static constexpr SquareMatrix Identity();
    };
    
    using Matrix2x2f = SquareMatrix<2, float>;
    using Matrix2x2d = SquareMatrix<2, double>;
    using Matrix3x3f = SquareMatrix<3, float>;
    using Matrix3x3d = SquareMatrix<3, double>;
    using Matrix4x4f = SquareMatrix<4, float>;
    using Matrix4x4d = SquareMatrix<4, double>;
    
    constexpr Matrix4x4f To3DMatrix(const Matrix2x2f& matrix2D, const Vec2& translation);
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Default constructor initializes all elements to 0. 
    //----------------------------------------------------------------------------------------------------
    template <int N, FloatingPointType Type>
    constexpr SquareMatrix<N, Type>::SquareMatrix()
    {
        memset(&(m[0][0]), 0, N * N * sizeof(float));
    }

    template <int N, FloatingPointType Type>
    constexpr SquareMatrix<N, Type>::SquareMatrix(const Type values[N * N])
    {
        NES_ASSERT(values != nullptr);
        //NES_ASSERTV((sizeof(values) / sizeof(values[0])) == (N * N), "Array of values must have size equal to N * N.");

        memcpy(&(m[0][0]), values, N * N * sizeof(Type));
    }
    
    template <int N, FloatingPointType Type>
    constexpr bool SquareMatrix<N, Type>::operator==(const SquareMatrix& other) const
    {
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                if (m[i][j] != other.m[i][j])
                return false;
            }
        }

        return true;
    }

    template <int N, FloatingPointType Type>
    constexpr bool SquareMatrix<N, Type>::operator!=(const SquareMatrix& other) const
    {
        return !(*this == other);
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type> SquareMatrix<N, Type>::operator+(const SquareMatrix& other) const
    {
        SquareMatrix result(*this);
        result += other;
        return result;
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type>& SquareMatrix<N, Type>::operator+=(const SquareMatrix& other)
    {
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                m[i][j] += other.m[i][j];
            }
        }
        
        return *this;
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type> SquareMatrix<N, Type>::operator-(const SquareMatrix& other) const
    {
        SquareMatrix result(*this);
        result -= other;
        return result;
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type>& SquareMatrix<N, Type>::operator-=(const SquareMatrix& other)
    {
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                m[i][j] -= other.m[i][j];
            }
        }
        
        return *this;
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type> SquareMatrix<N, Type>::operator*(const SquareMatrix& other) const
    {
        SquareMatrix result{};

        // 2x2
        if constexpr (N == 2)
        {
            // 1st Row * 1-2 Columns
            result.m[0][0] = (m[0][0] * other.m[0][0]) + (m[0][1] * other.m[1][0]);
            result.m[0][1] = (m[0][0] * other.m[0][1]) + (m[0][1] * other.m[1][1]);

            // 2nd Row * 1-2 Columns
            result.m[1][0] = (m[1][0] * other.m[0][0] + (m[1][1] * other.m[1][0]));
            result.m[1][1] = (m[1][0] * other.m[0][1] + (m[1][1] * other.m[1][1]));
        }

        // 3x3
        else if constexpr (N == 3)
        {
            // 1st Row * 1-3 Columns
            result.m[0][0] = (m[0][0] * other.m[0][0]) + (m[0][1] * other.m[0][1]) + (m[0][2] * other.m[0][2]);
            result.m[0][1] = (m[0][0] * other.m[1][0]) + (m[0][1] * other.m[1][1]) + (m[0][2] * other.m[1][2]);
            result.m[0][2] = (m[0][0] * other.m[2][0]) + (m[0][1] * other.m[2][1]) + (m[0][2] * other.m[2][2]);
            
            // 2nd Row * 1-3 Columns
            result.m[1][0] = (m[1][0] * other.m[0][0]) + (m[1][1] * other.m[0][1]) + (m[1][2] * other.m[0][2]);
            result.m[1][1] = (m[1][0] * other.m[1][0]) + (m[1][1] * other.m[1][1]) + (m[1][2] * other.m[1][2]);
            result.m[1][2] = (m[1][0] * other.m[2][0]) + (m[1][1] * other.m[2][1]) + (m[1][2] * other.m[2][2]);

            // 3rd Row * 1-3 Columns
            result.m[2][0] = (m[2][0] * other.m[0][0]) + (m[2][1] * other.m[0][1]) + (m[2][2] * other.m[0][2]);
            result.m[2][1] = (m[2][0] * other.m[1][0]) + (m[2][1] * other.m[1][1]) + (m[2][2] * other.m[1][2]);
            result.m[2][2] = (m[2][0] * other.m[2][0]) + (m[2][1] * other.m[2][1]) + (m[2][2] * other.m[2][2]);
        }

        // 4x4
        else if constexpr (N == 4)
        {
            // 1st Row * 1-4 Columns
            result.m[0][0] = (m[0][0] * other.m[0][0]) + (m[0][1] * other.m[0][1]) + (m[0][2] * other.m[0][2]) + (m[0][3] * other.m[0][3]);
            result.m[0][1] = (m[0][0] * other.m[1][0]) + (m[0][1] * other.m[1][1]) + (m[0][2] * other.m[1][2]) + (m[0][3] * other.m[1][3]);
            result.m[0][2] = (m[0][0] * other.m[2][0]) + (m[0][1] * other.m[2][1]) + (m[0][2] * other.m[2][2]) + (m[0][3] * other.m[2][3]);
            result.m[0][3] = (m[0][0] * other.m[3][0]) + (m[0][1] * other.m[3][1]) + (m[0][2] * other.m[3][2]) + (m[0][3] * other.m[3][3]);

            // 2nd Row * 1-4 Columns
            result.m[1][0] = (m[1][0] * other.m[0][0]) + (m[1][1] * other.m[0][1]) + (m[1][2] * other.m[0][2]) + (m[1][3] * other.m[0][3]);
            result.m[1][1] = (m[1][0] * other.m[1][0]) + (m[1][1] * other.m[1][1]) + (m[1][2] * other.m[1][2]) + (m[1][3] * other.m[1][3]);
            result.m[1][2] = (m[1][0] * other.m[2][0]) + (m[1][1] * other.m[2][1]) + (m[1][2] * other.m[2][2]) + (m[1][3] * other.m[2][3]);
            result.m[1][3] = (m[1][0] * other.m[3][0]) + (m[1][1] * other.m[3][1]) + (m[1][2] * other.m[3][2]) + (m[1][3] * other.m[3][3]);

            // 3rd Row * 1-4 Columns
            result.m[2][0] = (m[2][0] * other.m[0][0]) + (m[2][1] * other.m[0][1]) + (m[2][2] * other.m[0][2]) + (m[2][3] * other.m[0][3]);
            result.m[2][1] = (m[2][0] * other.m[1][0]) + (m[2][1] * other.m[1][1]) + (m[2][2] * other.m[1][2]) + (m[2][3] * other.m[1][3]);
            result.m[2][2] = (m[2][0] * other.m[2][0]) + (m[2][1] * other.m[2][1]) + (m[2][2] * other.m[2][2]) + (m[2][3] * other.m[2][3]);
            result.m[2][3] = (m[2][0] * other.m[3][0]) + (m[2][1] * other.m[3][1]) + (m[2][2] * other.m[3][2]) + (m[2][3] * other.m[3][3]);

            // 4th Row * 1-4 Columns
            result.m[3][0] = (m[2][0] * other.m[0][0]) + (m[2][1] * other.m[0][1]) + (m[2][2] * other.m[0][2]) + (m[3][3] * other.m[0][3]);
            result.m[3][1] = (m[2][0] * other.m[1][0]) + (m[2][1] * other.m[1][1]) + (m[2][2] * other.m[1][2]) + (m[3][3] * other.m[1][3]);
            result.m[3][2] = (m[2][0] * other.m[2][0]) + (m[2][1] * other.m[2][1]) + (m[2][2] * other.m[2][2]) + (m[3][3] * other.m[2][3]);
            result.m[3][3] = (m[2][0] * other.m[3][0]) + (m[2][1] * other.m[3][1]) + (m[2][2] * other.m[3][2]) + (m[3][3] * other.m[3][3]);
        }

        else
        {
            static_assert(false, "No multiplication implementation for this dimension.");
        }
        
        return result;
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type>& SquareMatrix<N, Type>::operator*=(const SquareMatrix& other)
    {
        *this = *this * other;
        return *this;
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type> SquareMatrix<N, Type>::operator*(const float scalar)
    {
        SquareMatrix result(*this);
        result *= scalar;
        return result;
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type>& SquareMatrix<N, Type>::operator*=(const float scalar)
    {
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                m[i][j] *= scalar;
            }
        }

        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      I need to support 2x2 and 4x4.
    //		
    ///		@brief : Attempt to Invert this Matrix. If it is non-invertible, then this will return false and
    ///             the Matrix will remain unchanged.
    //----------------------------------------------------------------------------------------------------
    template <int N, FloatingPointType Type>
    bool SquareMatrix<N, Type>::TryInvert()
    {
        const SquareMatrix<N, Type> copy = *this;
        
        // 2x2
        if constexpr (N == 2)
        {
            // The Inverse of a Matrix is on page 168 of my Math Textbook:
            const float determinant = CalculateDeterminant();
            if (determinant == 0.f)
            {
                return false;
            }

            const float invDeterminant = 1.0f / determinant;
            
            // The inverse determinant * the adjugate
            // Nice video explaining the process: https://www.youtube.com/watch?v=01c12NaUQDw
            m[0][0] = invDeterminant * copy.m[1][1];
            m[1][1] = invDeterminant * copy.m[0][0];

            m[0][1] = invDeterminant * -copy.m[0][1];
            m[1][0] = invDeterminant * -copy.m[1][0];
        }

        // 3x3
        else if constexpr (N == 3)
        {
            // The Inverse of a Matrix is on page 168 of my Math Textbook:
            const float determinant = CalculateDeterminant();
            if (determinant == 0.f)
            {
                return false;
            }

            const float invDeterminant = 1.0f / determinant;
            
            m[0][0] = invDeterminant * DifferenceOfProducts(copy.m[1][1], copy.m[2][2], copy.m[1][2], copy.m[2][1]);
            m[1][0] = invDeterminant * DifferenceOfProducts(copy.m[1][2], copy.m[2][0], copy.m[1][0], copy.m[2][2]);
            m[2][0] = invDeterminant * DifferenceOfProducts(copy.m[1][0], copy.m[2][1], copy.m[1][1], copy.m[2][0]);
            
            m[0][1] = invDeterminant * DifferenceOfProducts(copy.m[0][2], copy.m[2][1], copy.m[0][1], copy.m[2][2]);
            m[1][1] = invDeterminant * DifferenceOfProducts(copy.m[0][0], copy.m[2][2], copy.m[0][2], copy.m[2][0]);
            m[2][1] = invDeterminant * DifferenceOfProducts(copy.m[0][1], copy.m[2][0], copy.m[0][0], copy.m[2][1]);
            
            m[0][2] = invDeterminant * DifferenceOfProducts(copy.m[0][1], copy.m[1][2], copy.m[0][2], copy.m[1][1]);
            m[1][2] = invDeterminant * DifferenceOfProducts(copy.m[0][2], copy.m[1][0], copy.m[0][0], copy.m[1][2]);
            m[2][2] = invDeterminant * DifferenceOfProducts(copy.m[0][0], copy.m[1][1], copy.m[0][1], copy.m[1][0]);
        }

        // 4x4
        else if constexpr (N == 4)
        {
            // Calculate the Determinant of the 4 2x2 matrices.
            // I am looking at Unreal's implementation, because in my research, it seems that a lot of implementations try to avoid
            // calculating part of the determinant more than once. Honestly, it is a bit above my head, but I need to get things in
            // to move on for now.
            // File: UnrealMath.cpp, Line: 834
            float determinants[4]{};
            float tmp[4][4];

            tmp[0][0] = copy.m[2][2] * copy.m[3][3] - copy.m[2][3] * copy.m[3][2];
            tmp[0][1] = copy.m[1][2] * copy.m[3][3] - copy.m[1][3] * copy.m[3][2];
            tmp[0][2] = copy.m[1][2] * copy.m[2][3] - copy.m[1][3] * copy.m[2][2];

            tmp[1][0] = copy.m[2][2] * copy.m[3][3] - copy.m[2][3] * copy.m[3][2];
            tmp[1][1] = copy.m[0][2] * copy.m[3][3] - copy.m[0][3] * copy.m[3][2];
            tmp[1][2] = copy.m[0][2] * copy.m[2][3] - copy.m[0][3] * copy.m[2][2];
            
            tmp[2][0] = copy.m[1][2] * copy.m[3][3] - copy.m[1][3] * copy.m[3][2];
            tmp[2][1] = copy.m[0][2] * copy.m[3][3] - copy.m[0][3] * copy.m[3][2];
            tmp[2][2] = copy.m[0][2] * copy.m[1][3] - copy.m[0][3] * copy.m[1][2];

            tmp[3][0] = copy.m[1][2] * copy.m[2][3] - copy.m[1][3] * copy.m[2][2];
            tmp[3][1] = copy.m[0][2] * copy.m[2][3] - copy.m[0][3] * copy.m[2][2];
            tmp[3][2] = copy.m[0][2] * copy.m[1][3] - copy.m[0][3] * copy.m[1][2];

            determinants[0] = copy.m[1][1] * tmp[0][0] - copy.m[2][1] * tmp[0][1] + copy.m[3][1] * tmp[0][2];
            determinants[1] = copy.m[0][1] * tmp[1][0] - copy.m[2][1] * tmp[1][1] + copy.m[3][1] * tmp[1][2];
            determinants[2] = copy.m[0][1] * tmp[2][0] - copy.m[1][1] * tmp[2][1] + copy.m[3][1] * tmp[2][2];
            determinants[3] = copy.m[0][1] * tmp[3][0] - copy.m[1][1] * tmp[3][1] + copy.m[2][1] * tmp[3][2];

            const float determinant = copy.m[0][0] * determinants[0] - copy.m[1][0] * determinants[1] + copy.m[2][0] * determinants[2] + copy.m[3][0] * determinants[3];
            if (determinant == 0.f)
            {
                return false;
            }

            const float invDeterminant = 1.f / determinant;
            m[0][0] =  invDeterminant * determinants[0];
            m[0][1] = -invDeterminant * determinants[1];
            m[0][2] =  invDeterminant * determinants[2];
            m[0][3] = -invDeterminant * determinants[3];

            m[1][0] = -invDeterminant * (copy.m[1][0] * tmp[0][0] - copy.m[2][0] * tmp[0][1] + copy.m[3][0] * tmp[0][2]);
            m[1][1] =  invDeterminant * (copy.m[0][0] * tmp[1][0] - copy.m[2][0] * tmp[1][1] + copy.m[3][0] * tmp[1][2]);
            m[1][2] = -invDeterminant * (copy.m[0][0] * tmp[2][0] - copy.m[2][0] * tmp[2][1] + copy.m[3][0] * tmp[2][2]);
            m[1][3] =  invDeterminant * (copy.m[0][0] * tmp[3][0] - copy.m[2][0] * tmp[3][1] + copy.m[2][0] * tmp[3][2]);

            m[2][0] =  invDeterminant * (
                copy.m[1][0] * (copy.m[2][1] * copy.m[3][3] - copy.m[2][3] * copy.m[3][1]) -
                copy.m[2][0] * (copy.m[1][1] * copy.m[3][3] - copy.m[1][3] * copy.m[3][1]) +
                copy.m[3][0] * (copy.m[1][1] * copy.m[2][3] - copy.m[1][3] * copy.m[2][1])
                );
            m[2][1] = -invDeterminant * (
                copy.m[0][0] * (copy.m[2][1] * copy.m[3][3] - copy.m[1][3] * copy.m[3][1]) -
                copy.m[2][0] * (copy.m[0][1] * copy.m[3][3] - copy.m[0][3] * copy.m[3][1]) +
                copy.m[3][0] * (copy.m[0][1] * copy.m[2][3] - copy.m[0][3] * copy.m[2][1])
                );
            m[2][2] =  invDeterminant * (
                copy.m[0][0] * (copy.m[1][1] * copy.m[3][3] - copy.m[1][3] * copy.m[3][1]) -
                copy.m[1][0] * (copy.m[0][1] * copy.m[3][3] - copy.m[0][3] * copy.m[3][1]) +
                copy.m[3][0] * (copy.m[0][1] * copy.m[1][3] - copy.m[0][3] * copy.m[1][1])
                );
            m[2][3] = -invDeterminant * (
                copy.m[0][0] * (copy.m[1][1] * copy.m[2][3] - copy.m[1][3] * copy.m[2][1]) -
                copy.m[1][0] * (copy.m[0][1] * copy.m[2][3] - copy.m[0][3] * copy.m[2][1]) +
                copy.m[2][0] * (copy.m[0][1] * copy.m[1][3] - copy.m[0][3] * copy.m[1][1])
                );

            m[3][0] = -invDeterminant * (
                copy.m[0][0] * (copy.m[2][1] * copy.m[3][2] - copy.m[2][2] * copy.m[3][1]) -
                copy.m[1][0] * (copy.m[1][1] * copy.m[3][2] - copy.m[1][2] * copy.m[3][1]) +
                copy.m[3][0] * (copy.m[1][1] * copy.m[2][2] - copy.m[1][2] * copy.m[2][1])
                );
            m[3][1] =  invDeterminant * (
                copy.m[0][0] * (copy.m[2][1] * copy.m[3][2] - copy.m[2][2] * copy.m[3][1]) -
                copy.m[2][0] * (copy.m[0][1] * copy.m[3][2] - copy.m[0][2] * copy.m[3][1]) +
                copy.m[3][0] * (copy.m[0][1] * copy.m[2][2] - copy.m[0][2] * copy.m[2][1])
                );
            m[3][2] = -invDeterminant * (
                copy.m[0][0] * (copy.m[1][1] * copy.m[3][2] - copy.m[1][2] * copy.m[3][1]) -
                copy.m[1][0] * (copy.m[0][1] * copy.m[3][2] - copy.m[0][2] * copy.m[3][1]) +
                copy.m[3][0] * (copy.m[0][1] * copy.m[1][2] - copy.m[0][2] * copy.m[1][1])
                );
            m[3][3] =  invDeterminant * (
                copy.m[0][0] * (copy.m[1][1] * copy.m[2][2] - copy.m[1][2] * copy.m[2][1]) -
                copy.m[1][0] * (copy.m[0][1] * copy.m[2][2] - copy.m[0][2] * copy.m[2][1]) +
                copy.m[2][0] * (copy.m[0][1] * copy.m[1][2] - copy.m[0][2] * copy.m[1][1])
                );
        }

        else
        {
            static_assert(false, "No Inverse implementation for this Dimension.");
        }

        return true;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Attempt to get the inverse of this Matrix. If no inverse is possible, this will return
    ///              false and "result" will be equal to this Matrix.
    ///		@param result : The resulting inverse of the Matrix, or equal to the Matrix if no inverse is possible.
    //----------------------------------------------------------------------------------------------------
    template <int N, FloatingPointType Type>
    bool SquareMatrix<N, Type>::TryGetInverse(SquareMatrix<N, Type>& result) const
    {
        result = *this;
        return result.TryInvert();
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type>& SquareMatrix<N, Type>::Transpose()
    {
        *this = GetTranspose();
        return *this;
    }

    template <int N, FloatingPointType Type>
    SquareMatrix<N, Type> SquareMatrix<N, Type>::GetTranspose() const
    {
        SquareMatrix result;

        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                result.m[i][j] = m[j][i];
            }
        }
        
        return result;
    }
    
    template <int N, FloatingPointType Type>
    float SquareMatrix<N, Type>::CalculateDeterminant() const
    {
        // Page 162 of my Math Textbook: "3D Math Primer for Graphics and Game Development".

        // 2x2
        if constexpr (N == 2)
        {
            // Difference of the products of the two diagonals
            return (m[0][0] * m[1][1]) - (m[0][1] * m[1][0]);
        }

        // 3x3
        else if constexpr (N == 3)
        {
            float determinant = 0.0f;

            // Difference of the products of the 3 forward and 3 backward diagonals
            // An image can be found in the book that shows it clearly.
            determinant += (m[0][0] * m[1][1] * m[2][2]);
            determinant += (m[0][1] * m[1][2] * m[2][0]);
            determinant += (m[0][2] * m[1][0] * m[2][1]);

            determinant -= (m[0][0] * m[1][2] * m[2][1]);
            determinant -= (m[0][1] * m[1][0] * m[2][2]);
            determinant -= (m[0][2] * m[1][1] * m[2][0]);

            return determinant;
        }

        // 4x4
        else if constexpr (N == 4)
        {
            const float s0 = DifferenceOfProducts(m[0][0], m[1][1], m[1][0], m[0][1]);
            const float s1 = DifferenceOfProducts(m[0][0], m[1][2], m[1][0], m[0][2]);
            const float s2 = DifferenceOfProducts(m[0][0], m[1][3], m[1][0], m[0][3]);

            const float s3 = DifferenceOfProducts(m[0][1], m[1][2], m[1][1], m[0][2]);
            const float s4 = DifferenceOfProducts(m[0][1], m[1][3], m[1][1], m[0][3]);
            const float s5 = DifferenceOfProducts(m[0][2], m[1][3], m[1][2], m[0][3]);

            const float c0 = DifferenceOfProducts(m[2][0], m[3][1], m[3][0], m[2][1]);
            const float c1 = DifferenceOfProducts(m[2][0], m[3][2], m[3][0], m[2][2]);
            const float c2 = DifferenceOfProducts(m[2][0], m[3][3], m[3][0], m[2][3]);

            const float c3 = DifferenceOfProducts(m[2][1], m[3][2], m[3][1], m[2][2]);
            const float c4 = DifferenceOfProducts(m[2][1], m[3][3], m[3][1], m[2][3]);
            const float c5 = DifferenceOfProducts(m[2][2], m[3][3], m[3][2], m[2][3]);
            
            return (DifferenceOfProducts(s0, c5, s1, c4)
                + DifferenceOfProducts(s2, c3, -s3, c2)
                + DifferenceOfProducts(s5, c0, s4, c1));
        }

        else
        {
            static_assert(false, "Unhandled Determinant Dimension.");
            return 0.0f;
        }
    }

    template <int N, FloatingPointType Type>
    bool SquareMatrix<N, Type>::IsIdentity() const
    {
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                if (i == j && m[i][j] != 1.f)
                {
                    return false;
                }

                if (m[i][j] != 0.f)
                {
                    return false;
                }
            }
        }

        return true;
    }


    template <int N, FloatingPointType Type>
    constexpr SquareMatrix<N, Type> SquareMatrix<N, Type>::Zero()
    {
        SquareMatrix result{};
        memset(&(result.m[0][0]), 0, N * N * sizeof(float));
        return result;
    }

    template <int N, FloatingPointType Type>
    constexpr SquareMatrix<N, Type> SquareMatrix<N, Type>::Identity()
    {
        SquareMatrix result{};
        
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                result.m[i][j] = (i == j)? 1.f : 0.f;
            }
        }

        return result;
    }

    template <int N, FloatingPointType Type>
    std::string SquareMatrix<N, Type>::ToString() const
    {
        std::string result;
        result.reserve(static_cast<size_t>(N * N));

        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                result += std::to_string(m[i][j]) + " ";
            }
            
            result += "\n";
        }

        return result;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //		
    ///		@brief : Converts a 2x2 matrix, containing the Rotation and Scale, and a Translation into
    ///             a 3D representation.
    //----------------------------------------------------------------------------------------------------
    constexpr Matrix4x4f To3DMatrix(const Matrix2x2f& matrix2D, const Vec2& translation)
    {
        const float elements[16]
        {
            matrix2D.m[0][0], matrix2D.m[0][1], 0.f, 0.f,
            matrix2D.m[1][0], matrix2D.m[1][1], 0.f, 0.f,
            0.f , 0.f, 1.f, 0.f,
            translation.x, translation.y, 0.f, 1.f,
        };

        return Matrix4x4f(elements);
    }
}
