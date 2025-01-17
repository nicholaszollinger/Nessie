// Matrix.h
#pragma once
#include <span>

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
    //      Left off:
    //        - Need support for matrix multiplication of 3x3 and 4x4.
    //        - Need support for Inverse of 2x2 and 4x4.
    //		
    ///		@brief : A Square Matrix.
    ///		@tparam N : Dimension of the Square Matrix. Ex: N == 2 would be a 2x2 Matrix. 
    //----------------------------------------------------------------------------------------------------
    template <int N>
    struct SquareMatrix
    {
    private:
        float m_elements[N * N] {};

    public:
        using ConstRowType = std::span<const float, N>;
        using RowType = std::span<float, N>;

    public:
        constexpr SquareMatrix();
        constexpr SquareMatrix(const SquareMatrix&) = default;
        constexpr SquareMatrix(SquareMatrix&&) = default;
        constexpr SquareMatrix& operator=(const SquareMatrix&) = default;
        constexpr SquareMatrix& operator=(SquareMatrix&&) = default;
        
        // Equality
        constexpr bool operator==(const SquareMatrix& other) const;
        constexpr bool operator!=(const SquareMatrix& other) const;

        // Indexing Rows:
        constexpr ConstRowType operator[](int index) const;
        RowType operator[](int index);
        
        // Addition
        SquareMatrix operator+(const SquareMatrix& other) const;
        SquareMatrix& operator+=(const SquareMatrix& other);
        
        // Subtraction
        SquareMatrix operator-(const SquareMatrix& other) const;
        SquareMatrix& operator-=(const SquareMatrix& other);

        // Multiplication
        SquareMatrix operator*(const SquareMatrix& other) const;
        SquareMatrix& operator*=(const SquareMatrix& other);
        SquareMatrix operator*(const float scalar);
        SquareMatrix& operator*=(const float scalar);

        [[nodiscard]] const float* GetData() const { return m_elements; }
        [[nodiscard]] float* GetData() { return m_elements; }
        
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

    using Matrix2x2 = SquareMatrix<2>;
    using Matrix3x3 = SquareMatrix<3>;
    using Matrix4x4 = SquareMatrix<3>;
}

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    ///		@brief : Default constructor initializes all elements to 0. 
    //----------------------------------------------------------------------------------------------------
    template <int N>
    constexpr SquareMatrix<N>::SquareMatrix()
    {
        for (int i = 0; i < N * N; ++i)
        {
            m_elements[i] = 0.0f;
        }
    }
    
    template <int N>
    constexpr bool SquareMatrix<N>::operator==(const SquareMatrix& other) const
    {
        for (int i = 0; i < N * N; ++i)
        {
            if (m_elements[i] != other.m_elements[i])
                return false;
        }

        return true;
    }

    template <int N>
    constexpr bool SquareMatrix<N>::operator!=(const SquareMatrix& other) const
    {
        return !(*this == other);
    }

    template <int N>
    constexpr typename SquareMatrix<N>::ConstRowType SquareMatrix<N>::operator[](const int index) const
    {
        NES_ASSERT(index >= 0 && index < N);
        return ConstRowType(&m_elements[index * N], N);
    }

    template <int N>
    typename SquareMatrix<N>::RowType SquareMatrix<N>::operator[](int index)
    {
        NES_ASSERT(index >= 0 && index < N);
        return RowType(&m_elements[index * N], N);
    }

    template <int N>
    SquareMatrix<N> SquareMatrix<N>::operator+(const SquareMatrix& other) const
    {
        SquareMatrix result(*this);
        result += other;
        return result;
    }

    template <int N>
    SquareMatrix<N>& SquareMatrix<N>::operator+=(const SquareMatrix& other)
    {
        for (int i = 0; i < N * N; ++i)
        {
            m_elements[i] += other.m_elements[i];
        }
        
        return *this;
    }

    template <int N>
    SquareMatrix<N> SquareMatrix<N>::operator-(const SquareMatrix& other) const
    {
        SquareMatrix result(*this);
        result -= other;
        return result;
    }

    template <int N>
    SquareMatrix<N>& SquareMatrix<N>::operator-=(const SquareMatrix& other)
    {
        for (int i = 0; i < N * N; ++i)
        {
            m_elements[i] -= other.m_elements[i];
        }
        
        return *this;
    }

    template <int N>
    SquareMatrix<N> SquareMatrix<N>::operator*(const SquareMatrix& other) const
    {
        SquareMatrix result(*this);
        return result *= other;
    }

    template <int N>
    SquareMatrix<N>& SquareMatrix<N>::operator*=(const SquareMatrix& other)
    {
        // 2x2 implementation
        if constexpr (N == 2)
        {
            // a11 = a11b11 + a12b21
            const float a11 = (m_elements[0] * other.m_elements[0]) + (m_elements[1] * other.m_elements[2]);

            // a12 = a11b12 + a12b22
            const float a12 = (m_elements[0] * other.m_elements[1]) + (m_elements[1] * other.m_elements[3]);

            // a21 = a21b11 + a22b21
            const float a21 = (m_elements[2] * other.m_elements[0] + (m_elements[3] * other.m_elements[2]));

            // a22 = a21b12 + a22b22
            const float a22 = (m_elements[2] * other.m_elements[1] + (m_elements[3] * other.m_elements[3]));

            m_elements[0] = a11;
            m_elements[1] = a12;
            m_elements[2] = a21;
            m_elements[3] = a22;
        }

        // [TODO]: 3x3 and 4x4

        else
        {
            static_assert(false, "I haven't implemented multiplication for this dimension yet.");
        }

        return *this;
    }

    template <int N>
    SquareMatrix<N> SquareMatrix<N>::operator*(const float scalar)
    {
        SquareMatrix result(*this);
        result *= scalar;
        return result;
    }

    template <int N>
    SquareMatrix<N>& SquareMatrix<N>::operator*=(const float scalar)
    {
        for (int i = 0; i < N * N; ++i)
        {
            m_elements[i] *= scalar;
        }

        return *this;
    }

    //----------------------------------------------------------------------------------------------------
    //		NOTES:
    //      I need to support 2x2 and 4x4.
    //		
    ///		@brief : Attempt to Invert this Matrix. If it is invertible, then this will return false and
    ///             the Matrix will remain unchanged.
    //----------------------------------------------------------------------------------------------------
    template <int N>
    bool SquareMatrix<N>::TryInvert()
    {
        // The Inverse of a Matrix is on page 168 of my Math Textbook:
        const float determinant = CalculateDeterminant();
        if (determinant == 0.f)
        {
            return false;
        }

        const float invDeterminant = 1.0f / determinant;
        const SquareMatrix<N> copy = *this;

        // 3x3
        if constexpr (N == 3)
        {
            // Get the Rows
            const auto r0 = copy[0];
            const auto r1 = copy[1];
            const auto r2 = copy[2];
            
            m_elements[0] = invDeterminant * DifferenceOfProducts(r1[1], r2[2], r1[2], r2[1]);
            m_elements[3] = invDeterminant * DifferenceOfProducts(r1[2], r2[0], r1[0], r2[2]);
            m_elements[6] = invDeterminant * DifferenceOfProducts(r1[0], r2[1], r1[1], r2[0]);
            
            m_elements[1] = invDeterminant * DifferenceOfProducts(r0[2], r2[1], r0[1], r2[2]);
            m_elements[4] = invDeterminant * DifferenceOfProducts(r0[0], r2[2], r0[2], r2[0]);
            m_elements[7] = invDeterminant * DifferenceOfProducts(r0[1], r2[0], r0[0], r2[1]);
            
            m_elements[2] = invDeterminant * DifferenceOfProducts(r0[1], r1[2], r0[2], r1[1]);
            m_elements[5] = invDeterminant * DifferenceOfProducts(r0[2], r1[0], r0[0], r1[2]);
            m_elements[8] = invDeterminant * DifferenceOfProducts(r0[0], r1[1], r0[1], r1[0]);
        }

        else
        {
            static_assert(false, "No Inverse Method available for this Dimension.");
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
    template <int N>
    bool SquareMatrix<N>::TryGetInverse(SquareMatrix<N>& result) const
    {
        result = *this;
        return result.TryInvert();
    }

    template <int N>
    SquareMatrix<N>& SquareMatrix<N>::Transpose()
    {
        *this = GetTranspose();
        return *this;
    }

    template <int N>
    SquareMatrix<N> SquareMatrix<N>::GetTranspose() const
    {
        SquareMatrix result;

        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                int destIndex = i * N + j;
                int sourceIndex = j * N + i;
                result.m_elements[destIndex] = m_elements[sourceIndex];
            }
        }
        
        return result;
    }
    
    template <int N>
    float SquareMatrix<N>::CalculateDeterminant() const
    {
        // Page 162 of my Math Textbook: "3D Math Primer for Graphics and Game Development".
        const SquareMatrix<N>& m = *this;

        // 2x2
        if constexpr (N == 2)
        {
            // Get the Rows:
            const auto r0 = m[0];
            const auto r1 = m[1];

            // Difference of the products of the two diagonals
            return DifferenceOfProducts(r0[0], r1[1], r0[1], r1[0]);
        }

        // 3x3
        else if constexpr (N == 3)
        {
            float determinant = 0.0f;

            // Get the Rows:
            const auto r0 = m[0];
            const auto r1 = m[1];
            const auto r2 = m[2];

            // Difference of the products of the 3 forward and 3 backward diagonals
            // An image can be found in the book that shows it clearly.
            determinant += (r0[0] * r1[1] * r2[2]);
            determinant += (r0[1] * r1[2] * r2[0]);
            determinant += (r0[2] * r1[0] * r2[1]);

            determinant -= (r0[0] * r1[2] * r2[1]);
            determinant -= (r0[1] * r1[1] * r2[2]);
            determinant -= (r0[2] * r1[0] * r2[0]);

            return determinant;
        }

        // 4x4
        else if constexpr (N == 4)
        {
            // Get the Rows:
            const auto r0 = m[0];
            const auto r1 = m[1];
            const auto r2 = m[2];
            const auto r3 = m[3];
            
            const float s0 = DifferenceOfProducts(r0[0], r1[1], r1[0], r0[1]);
            const float s1 = DifferenceOfProducts(r0[0], r1[2], r1[0], r0[2]);
            const float s2 = DifferenceOfProducts(r0[0], r1[3], r1[0], r0[3]);

            const float s3 = DifferenceOfProducts(r0[1], r1[2], r1[1], r0[2]);
            const float s4 = DifferenceOfProducts(r0[1], r1[3], r1[1], r0[3]);
            const float s5 = DifferenceOfProducts(r0[2], r1[3], r1[2], r0[3]);

            const float c0 = DiferenceOfProducts(r2[0], r3[1], r3[0], r2[1]);
            const float c1 = DiferenceOfProducts(r2[0], r3[2], r3[0], r2[2]);
            const float c2 = DiferenceOfProducts(r2[0], r3[3], r3[0], r2[3]);

            const float c3 = DiferenceOfProducts(r2[1], r3[2], r3[1], r2[2]);
            const float c4 = DiferenceOfProducts(r2[1], r3[3], r3[1], r2[3]);
            const float c5 = DiferenceOfProducts(r2[2], r3[3], r3[2], r2[3]);
            
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

    template <int N>
    bool SquareMatrix<N>::IsIdentity() const
    {
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                const int index = i * N + j;
                if (i == j && m_elements[index] != 1.f)
                {
                    return false;
                }

                if (m_elements[index] != 0.f)
                {
                    return false;
                }
            }
        }

        return true;
    }


    template <int N>
    constexpr SquareMatrix<N> SquareMatrix<N>::Zero()
    {
        SquareMatrix result{};
        
        for (int i = 0; i < N * N; ++i)
        {
            result.m_elements[i] = 0;
        }

        return result;
    }

    template <int N>
    constexpr SquareMatrix<N> SquareMatrix<N>::Identity()
    {
        SquareMatrix result{};
        
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                const int index = i * N + j;
                result.m_elements[index] = (i == j)? 1.f : 0.f;
            }
        }

        return result;
    }

    template <int N>
    std::string SquareMatrix<N>::ToString() const
    {
        std::string result;
        result.reserve(static_cast<size_t>(N * N));

        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                const int index = i * N + j;
                result += std::to_string(m_elements[index]) + " ";
            }
            
            result += "\n";
        }

        return result;
    }
}
