// InternalEdgeRemovingCollector.h
#pragma once

#include "Core/Memory/STLLocalAllocator.h"
#include "Physics/Collision/CollisionSolver.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Removes edges from collision results. Can be used to filter out 'ghost collisions'.
    ///     Based on: Contact generation for meshes - Pierre Terdiman (https://www.codercorner.com/MeshContacts.pdf)
    ///
    /// @note : This call requires that CollideSettingsBase::m_activeEdgeMode == EActiveEdgeMode::CollideWithAll
    ///     and CollideSettingsBase::m_collectFacesMode == ECollectFacesMode::CollectFaces.
    //----------------------------------------------------------------------------------------------------
    class InternalEdgeRemovingCollector : public CollideShapeCollector
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Constructor that configures a collector to be called with all the results that do not
        ///     hit internal edges.
        //----------------------------------------------------------------------------------------------------
        explicit        InternalEdgeRemovingCollector(CollideShapeCollector& chainedCollector);

        //----------------------------------------------------------------------------------------------------
        /// @brief : See: CollideShapeCollector::Reset. 
        //----------------------------------------------------------------------------------------------------
        virtual void    Reset() override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : See: CollideShapeCollector::OnBody. 
        //----------------------------------------------------------------------------------------------------
        virtual void    OnBody(const Body& body) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : See: CollideShapeCollector::OnBodyEnd. 
        //----------------------------------------------------------------------------------------------------
        virtual void    OnBodyEnd() override;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : See: CollideShapeCollector::AddHit. 
        //----------------------------------------------------------------------------------------------------
        virtual void    AddHit(const ResultType& result) override;

        //----------------------------------------------------------------------------------------------------
        /// @brief : After all hits have been added, call this function to process the delayed results. 
        //----------------------------------------------------------------------------------------------------
        void            Flush();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Version of CollisionSolver::CollideShapeVsShape that removes internal edges.
        //----------------------------------------------------------------------------------------------------
        static void     CollideShapeVsShape(const Shape* pShape1, const Shape* pShape2, const Vec3 scale1, const Vec3 scale2, const Mat44& centerOfMassTransform1, const Mat44& centerOfMassTransform2, const SubShapeIDCreator& subShapeIDCreator1, const SubShapeIDCreator& subShapeIDCreator2, const CollideShapeSettings& settings, CollideShapeCollector& inCollector, const ShapeFilter& shapeFilter = {});
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Check if a vertex is voided.
        //----------------------------------------------------------------------------------------------------
        inline bool     IsVoided(const SubShapeID& subShapeID, const Vec3 vec) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add all vertices of a face to the voided features. 
        //----------------------------------------------------------------------------------------------------
        inline void     VoidFeatures(const CollideShapeResult& result);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Call the chained collector. 
        //----------------------------------------------------------------------------------------------------
        inline void     Chain(const CollideShapeResult& result);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Call the chained collector and void all features of 'result' 
        //----------------------------------------------------------------------------------------------------
        inline void     ChainAndVoid(const CollideShapeResult& result);

    private:
        static constexpr uint kMaxLocalDelayedResults = 32;
        static constexpr uint kMaxLocalVoidedFeatures = 128;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : This algorithm tests a convex shape (shape 1) against a set of polygons (shape 2).
        ///     This assumption doesn't hold if the shape we're testing is a compound shape, so we must also
        ///     store the sub shape ID and ignore voided features that belong to another sub shape ID.
        //----------------------------------------------------------------------------------------------------
        struct Voided
        {
            Float3      m_feature;      /// Feature that is voided (of shape 2). Read with Vec3::LoadFloat3Unsafe so must not be the last member.
            SubShapeID  m_subShapeID;   /// Sub shape ID of the shape that is colliding against the feature (of shape 1).
        };

        CollideShapeCollector& m_chainedCollector;
        std::vector<Voided, STLLocalAllocator<Voided, kMaxLocalVoidedFeatures>> m_voidedFeatures;
        std::vector<CollideShapeResult, STLLocalAllocator<CollideShapeResult, kMaxLocalDelayedResults>> m_delayedResults;
    };
}