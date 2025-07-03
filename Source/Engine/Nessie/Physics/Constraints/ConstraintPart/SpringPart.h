// SpringPart.h
#pragma once
#include "Nessie/Math/Generic.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Class used in other constraint parts to calculate the required bias factor in the lagrange
    ///     multiplier for creating springs.
    //----------------------------------------------------------------------------------------------------
    class SpringPart
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Turn off the spring and set a bias only.
        ///	@param bias : Bias term (b) for the constraint impulse: lambda = J v + b
        //----------------------------------------------------------------------------------------------------
        inline void CalculateSpringPropertiesWithBias(float bias)
        {
            m_softness = 0.f;
            m_bias = bias;
        }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the spring properties with spring Stiffness (k) and damping (c), this is based on the spring equation: F = -k * x - c * v
        ///	@param deltaTime : Time step.
        ///	@param invEffectiveMass : Inverse of the effective mass K.
        ///	@param bias : Bias term (b) for the constraint impulse: lambda = J v + b
        ///	@param inC : Value of the contraint equation (C). Set to zero if you don't want to drive the constraint
        ///     to zero with a spring.
        ///	@param frequency : Oscillation frequency (Hz). Set to zero if you don't want to drive the constraint to zero with a spring.
        ///	@param damping : Spring damping coefficient (c). Set to zero if you don't want to drive the constraint to zero with a spring.
        ///	@param outEffectiveMass : On return, this contains the new effective mass K^-1.
        //----------------------------------------------------------------------------------------------------
        inline void CalculateSpringPropertiesWithFrequencyAndDamping(const float deltaTime, const float invEffectiveMass, const float bias, const float inC, const float frequency, const float damping, float& outEffectiveMass)
        {
            outEffectiveMass = 1.f / invEffectiveMass;

            if (frequency > 0.f)
            {
                // Calculate the angular frequency.
                const float omega = 2.f * math::Pi<float>() * frequency;

                // Calculate the spring stiffness k and damping constant c (page 45)
                const float k = outEffectiveMass * math::Squared(omega);
                const float c = 2.f * outEffectiveMass * damping * omega;

                CalculateSpringPropertiesHelper(deltaTime, invEffectiveMass, bias, inC, k, c, outEffectiveMass);
            }
            else
            {
                CalculateSpringPropertiesWithBias(bias);
            }
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculate the spring properties with spring Stiffness (k) and damping (c), this is based on the spring equation: F = -k * x - c * v
        ///	@param deltaTime : Time step.
        ///	@param invEffectiveMass : Inverse of the effective mass K.
        ///	@param bias : Bias term (b) for the constraint impulse: lambda = J v + b
        ///	@param inC : Value of the contraint equation (C). Set to zero if you don't want to drive the constraint
        ///     to zero with a spring.
        ///	@param stiffness : Spring stiffness (k). Set to zero if you don't want to drive the constraint to zero with a spring.
        ///	@param damping : Spring damping coefficient (c). Set to zero if you don't want to drive the constraint to zero with a spring.
        ///	@param outEffectiveMass : On return, this contains the new effective mass K^-1.
        //----------------------------------------------------------------------------------------------------
        inline void CalculateSpringPropertiesWithStiffnessAndDamping(const float deltaTime, const float invEffectiveMass, const float bias, const float inC, const float stiffness, const float damping, float& outEffectiveMass)
        {
            if (stiffness > 0.f)
            {
                CalculateSpringPropertiesHelper(deltaTime, invEffectiveMass, bias, inC, stiffness, damping, outEffectiveMass);
            }
            else
            {
                outEffectiveMass = 1.f / invEffectiveMass;
                CalculateSpringPropertiesWithBias(bias);
            }
        }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if the spring is active.
        //----------------------------------------------------------------------------------------------------
        inline bool IsActive() const { return m_softness != 0.f; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the total bias b, including supplied bias and bias for spring: lambda = J v + b
        //----------------------------------------------------------------------------------------------------
        inline float GetBias(const float totalLambda) const
        {
            // Remainder of post by Erin Catto: http://www.bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=4&t=1354
            //
            // Each iteration we are not computing the whole impulse, we are computing an increment to the impulse and we are updating the velocity.
            // Also, as we solve each constraint we get a perfect v2, but then some other constraint will come along and mess it up.
            // So we want to patch up the constraint while acknowledging the accumulated impulse and the damaged velocity.
            // To help with that we use P for the accumulated impulse and lambda as the update. Mathematically we have:
            //
            // M * (v2new - v2damaged) = J^T * lambda
            // J * v2new + softness * (total_lambda + lambda) + b = 0
            //
            // If we solve this we get:
            //
            // v2new = v2damaged + M^-1 * J^T * lambda
            // J * (v2damaged + M^-1 * J^T * lambda) + softness * total_lambda + softness * lambda + b = 0
            //
            // (J * M^-1 * J^T + softness) * lambda = -(J * v2damaged + softness * total_lambda + b)
            //
            // So our lagrange multiplier becomes:
            //
            // lambda = -K^-1 (J v + softness * total_lambda + b)
            //
            // So we return the bias: softness * total_lambda + b
            return m_softness * totalLambda + m_bias;
        }

    private:
        NES_INLINE void CalculateSpringPropertiesHelper(const float deltaTime, const float invEffectiveMass, const float bias, const float inC, const float stiffness, const float damping, float& outEffectiveMass)
        {
            // Soft constraints as per: Soft Constraints: Reinventing The Spring - Erin Catto - GDC 2011
            
            // Note that the calculation of beta and gamma below are based on the solution of an implicit Euler integration scheme.
            // This scheme is unconditionally stable but has built in damping, so even when you set the damping ratio to 0, there will still
            // be damping. See page 16 and 32.

            // Calculate softness (gamma in the slides)
            // See page 34 and note that the gamma needs to be divided by delta time since we're working with impulses rather than forces:
            // softness = 1 / (dt * (c + dt * k))
            // Note that the spring stiffness is k and the spring damping is c
            m_softness = 1.f / (deltaTime * (damping + deltaTime * stiffness));

            // Calculate bias factor (baumgarte stabilization):
            // beta = dt * k / (c + dt * k) = dt * k^2 * softness
            // b = beta / dt * C = dt * k * softness * C
            m_bias = bias + deltaTime * stiffness * m_softness * inC;

            // Update the effective mass, see post by Erin Catto: http://www.bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=4&t=1354
            //
            // Newton's Law:
            // M * (v2 - v1) = J^T * lambda
            //
            // Velocity constraint with softness and Baumgarte:
            // J * v2 + softness * lambda + b = 0
            //
            // where b = beta * C / dt
            //
            // We know everything except v2 and lambda.
            //
            // First solve Newton's law for v2 in terms of lambda:
            //
            // v2 = v1 + M^-1 * J^T * lambda
            //
            // Substitute this expression into the velocity constraint:
            //
            // J * (v1 + M^-1 * J^T * lambda) + softness * lambda + b = 0
            //
            // Now collect coefficients of lambda:
            //
            // (J * M^-1 * J^T + softness) * lambda = - J * v1 - b
            //
            // Now we define:
            //
            // K = J * M^-1 * J^T + softness
            //
            // So our new effective mass is K^-1
            outEffectiveMass = 1.f / (invEffectiveMass + m_softness);
        }
        
        float m_bias = 0.f;
        float m_softness = 0.f;
    };
}
