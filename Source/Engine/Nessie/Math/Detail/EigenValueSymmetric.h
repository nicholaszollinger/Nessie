// EigenValueSymmetric.h
#pragma once
#include "Nessie/Debug/Assert.h"
#include "Nessie/Math/FPException.h"
#include "Nessie/Math/Generic.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    //	NOTES:
    //  Honestly this is over my head at the moment, taken entirely for jolt physics, with math-style
    //  variable naming :(...
    //		
    /// @brief : Function to determine the eigen vectors and values of a square, real, symmetric matrix by
    ///     Jacobi transformations. This method is most suitable when MatrixTyep::N is <10.
    ///
    ///     An Eigen vector is a vector v in which Av = Lv; where:
    ///         * A = Square Matrix
    ///         * L = (lambda) is a non-zero constant value.
    ///
    /// @see https://en.wikipedia.org/wiki/Eigenvalues_and_eigenvectors
    /// @see https://numerical.recipes/book.html : "11.1 Jacobi Transformations of a Symmetric Matrix".
    ///
    ///	@param matrix : Square Matrix.
    ///	@param outEigenVectors : Output that whose columns contain the normalized eigen vectors. Must be == identity
    ///     before the function call.
    ///	@param outEigenValues : Output that will contain the eigen values.
    //----------------------------------------------------------------------------------------------------
    template <typename VectorType, typename MatrixType>
    bool EigenValueSymmetric(const MatrixType& matrix, MatrixType& outEigenVectors, VectorType& outEigenValues)
    {
        // This algorithm can generate infinite values - which are handled.
        [[maybe_unused]] FPExceptionDisableInvalid invalid;
        
        static_assert(MatrixType::N == VectorType::N, "Matrix and Vector types must have the same order of N!");
        static constexpr int kMaxNumSweeps = 50;

        // Get the matrix in a so we can mess with it
        MatrixType matCopy = matrix;
        VectorType b;
        VectorType z;

        for (size_t i = 0; i < MatrixType::N; ++i)
        {
            // Initialize b to the diagonal of the matCopy.
            b[i] = matCopy[i][i];

            // Initialize the output to the diagonal of matCopy
            outEigenValues[i] = matCopy[i][i];

            // Reset Z
            z[i] = 0.f;
        }

        for (int sweep = 0; sweep < kMaxNumSweeps; ++sweep)
        {
            // Get the sum of the off-diagonal elements of matCopy
            float sum = 0.f;
            for (size_t row = 0; row < MatrixType::N - 1; ++row)
            {
                for (size_t column = row + 1; column < MatrixType::N; ++column)
                {
                    sum += math::Abs(matCopy[column][row]);
                }
            }
            const float averageSum = sum / static_cast<float>(math::Squared(MatrixType::N));

            // Normal return, convergence to machine underflow
            if (averageSum < FLT_MIN) // Original code: sum == 0.0f, when the average is denormal, we also consider it machine underflow
            {
                // Sanity checks:
                #if NES_ASSERTS_ENABLED
                    for (size_t c = 0; c < MatrixType::N; ++c)
                    {
                        // Check if the eigenvector is normalized
                        NES_ASSERT(outEigenVectors[c].IsNormalized());

                        // Check if matrix * eigenvector = eigenvalue * eigenvector
                        VectorType matEigenVec = matrix * outEigenVectors[c];
                        VectorType eigenValEigenVec = outEigenValues[c] * outEigenVectors[c];
                        NES_ASSERT(matEigenVec.IsClose(eigenValEigenVec, math::Max(matEigenVec.LengthSqr(), eigenValEigenVec.LengthSqr()) * 1.0e-6f));
                    }
                #endif
                
                // Success
                return true;
            }

            // On the first 3 sweeps use a fraction of the sum of the off diagonal elements as threshold.
            // Note that we pick a minimum threshold of FLT_MIN because dividing by a denormalized number is likely to
            // result in infinity.
            const float threshold = sweep < 4? 0.2f * averageSum : FLT_MIN;

            for (size_t row = 0; row < MatrixType::N - 1; ++row)
            {
                for (size_t column = row + 1; column < MatrixType::N; ++column)
                {
                    float& copyPQ = matCopy[column][row];
                    float& eigenValP = outEigenValues[row];
                    float& eigenValQ = outEigenValues[column];

                    float absPQ = math::Abs(copyPQ);
                    float g = 100.f * absPQ;

                    // After 4 sweeps, skip the rotation if the off-diagonal element is small.
                    if (sweep > 4
                        && math::Abs(eigenValP) + g == math::Abs(eigenValP)
                        && math::Abs(eigenValQ) + g == math::Abs(eigenValQ))
                    {
                        copyPQ = 0.f;
                    }
                    else if (absPQ > threshold)
                    {
                        float h = eigenValQ - eigenValP;
                        const float absH = math::Abs(h);

                        float t;
                        if (absH + g == absH)
                        {
                            t = copyPQ / h;
                        }
                        else
                        {
                            // Warning: Can become infinite if copy[column][row] is very small which may trigger an invalid
                            // float exception.
                            const float theta = 0.5f * h / copyPQ;

                            // If theta becomes inf, t will be 0, so the infinite is not a problem for the algorithm. 
                            t = 1.f / (math::Abs(theta) + std::sqrt(1.f + theta * theta));

                            if (theta < 0.f)
                                t = -t;
                        }

                        const float c = 1.f / std::sqrt(1.f + t * t);
                        const float s = t * c;
                        const float tau = s / (1.f + c);
                        h = t * copyPQ;

                        copyPQ = 0.f;

                        z[row] -= h;
                        z[column] += h;

                        eigenValP -= h;
                        eigenValQ += h;

                        #define NES_EVS_ROTATE(mat, row1, column1, row2, column2)   \
                            g = mat[column1][row1],							        \
                            h = mat[column2][row2],							        \
                            mat[column1][row1] = g - s * (h + g * tau),		        \
                            mat[column2][row2] = h + s * (g - h * tau)

                        size_t j;
                        for (j = 0; j < row; ++j)
                        {
                            NES_EVS_ROTATE(matCopy, j, row, j, column);
                        }
                        
                        for (j = row + 1; j < column; ++j)
                        {
                            NES_EVS_ROTATE(matCopy, row, j, j, column);
                        }
                            
                        for (j = column + 1; j < MatrixType::N; ++j)
                        {
                            NES_EVS_ROTATE(matCopy, row, j, column, j);
                        }
                        
                        for (j = 0; j < MatrixType::N; ++j)
                        {
                            NES_EVS_ROTATE(outEigenVectors, j, row, j, column);
                        }

                        #undef NES_EVS_ROTATE
                    }
                }

                // Update eigen values with the sum of ta_PQ and reinitialize z
                for (size_t i = 0; i < MatrixType::N; ++i)
                {
                    b[i] += z[i];
                    outEigenValues[i] = b[i];
                    z[i] = 0.f;
                }
            }
        }

        NES_ASSERT(false, "Too many iterations!");
        return false;
    }
    
}
