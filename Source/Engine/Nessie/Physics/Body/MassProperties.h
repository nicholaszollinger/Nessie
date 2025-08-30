// MassProperties.h
#pragma once
#include "Nessie/Math/Math.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    // [TODO]: This should be serializable.
    /// @brief : Describes the mass and inertia properties of a body. Used during body construction only.  
    //----------------------------------------------------------------------------------------------------
    struct MassProperties
    {
        /// Inertia tensor of the shape (kg m^2)
        Mat44           m_inertia   = Mat44::Zero();

        /// Mass of the shape (kg).
        float           m_mass      = 0.f;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Using eigen composition, decompose the inertia tensor into a diagonal matrix D and a right-handed
        ///     rotation matrix R so that the inertia tensor is R * D * R^-1
        /// @see https://en.wikipedia.org/wiki/Moment_of_inertia section 'Principal axes'
        ///	@param outRotation : The rotation matrix R. 
        ///	@param outDiagonal : The diagonal of the diagonal matrix D.
        ///	@returns : True if successful.
        //----------------------------------------------------------------------------------------------------
        bool            DecomposePrincipalMomentsOfInertia(Mat44& outRotation, Vec3& outDiagonal) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the mass and inertia of a box with edge size = "boxSize" and density = "density".
        //----------------------------------------------------------------------------------------------------
        void            SetMassAndInertiaOfSolidBox(const Vec3& boxSize, const float density);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the mass and scale the inertia tensor to match the mass. 
        //----------------------------------------------------------------------------------------------------
        void            ScaleToMass(const float mass);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Rotate the inertia by 3x3 matrix rotation 
        //----------------------------------------------------------------------------------------------------
        void            Rotate(const Mat44& rotation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Translate the inertia .
        //----------------------------------------------------------------------------------------------------
        void            Translate(const Vec3& translation);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Scale the mass and inertia by "scale". Note that elements can be < 0 to flip the shape. 
        //----------------------------------------------------------------------------------------------------
        void            Scale(const Vec3& scale);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Calculates the size of a solid box that has an inertia tensor diagonal = "inertiaDiagonal"
        //----------------------------------------------------------------------------------------------------
        static Vec3     GetEquivalentSolidBoxSize(const float mass, const Vec3& inertiaDiagonal);
    };
}