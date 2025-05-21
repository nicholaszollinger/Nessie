// CollisionCollector.h
#pragma once
#include <cstdint>

#include "Core/QuickSort.h"
#include "Debug/Assert.h"
#include "Math/Generic.h"

namespace nes
{
    class Body;
    class TransformedShape;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Traits to use when Casting a Ray. For Rays, the early out fraction is the fraction along
    ///     then line to order hits.
    //----------------------------------------------------------------------------------------------------
    struct CollisionCollectorTraitsCastRay
    {
        /// Furthest hit: Fraction is 1 + epsilon.
        static constexpr float kInitialEarlyOutFraction = 1.f + math::PrecisionDelta<float>();

        /// Closest hit: Fraction is 0.
        static constexpr float kShouldEarlyOutFraction = 0.f; 
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Traits to use for Casting a Shape. For Rays, the early out fraction is the fraction along
    ///     then line to order hits.
    //----------------------------------------------------------------------------------------------------
    struct CollisionCollectorTraitsCastShape
    {
        /// Furthest hit: Fraction is 1 + epsilon.
        static constexpr float kInitialEarlyOutFraction = 1.f + math::PrecisionDelta<float>();

        /// Deepest hit: Penetration is infinite.
        static constexpr float kShouldEarlyOutFraction = std::numeric_limits<float>::min();
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Traits to use for Colliding a Shape. For Shape collisions, we use -penetration depth to
    ///     order hits.
    //----------------------------------------------------------------------------------------------------
    struct CollisionCollectorTraitsCollideShape
    {
        /// Most Shallow Hit: Separation is infinite.
        static constexpr float kInitialEarlyOutFraction = std::numeric_limits<float>::max();

        /// Deepest hit: Penetration is infinite.
        static constexpr float kShouldEarlyOutFraction = std::numeric_limits<float>::min();
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Traits to use for CollidePoint.
    //----------------------------------------------------------------------------------------------------
    using CollisionCollectorTraitsCollidePoint = CollisionCollectorTraitsCollideShape;

    template <typename Type>
    concept ValidCollisionCollectorTraitsType = requires()
    {
        std::same_as<decltype(Type::kInitialEarlyOutFraction), float>; 
        std::same_as<decltype(Type::kShouldEarlyOutFraction), float>; 
    };
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : Interface for collecting multiple Collision Results from a Query.
    ///	@tparam ResultTypeArg : Type of Result that this 
    ///	@tparam TraitsType : Traits that determine the Early Out Fraction.
    //----------------------------------------------------------------------------------------------------
    template <typename ResultTypeArg, ValidCollisionCollectorTraitsType TraitsType>
    class CollisionCollector
    {
        /// The early out fraction determine the fraction below which the collector is still
        /// accepting a hit. This is used to reduce the amount of work.
        float m_earlyOutFraction = TraitsType::kInitialEarlyOutFraction;

        /// Set by Collision Detection functions to the current TransformedShape of the Body
        /// that we're colliding against before calling the AddHit function.
        const TransformedShape* m_pContext = nullptr;
        
    public:
        using ResultType = ResultTypeArg;

        CollisionCollector() = default;
        CollisionCollector(const CollisionCollector& other) = default;
        virtual ~CollisionCollector() = default;

        template <typename ResultTypeArg2>
        explicit CollisionCollector(const CollisionCollector<ResultTypeArg2, TraitsType>& other) : m_earlyOutFraction(other.m_earlyOutFraction), m_pContext(other.m_pContext) {}
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : If you want to reuse this Collector, call Reset before performing another Query.
        //----------------------------------------------------------------------------------------------------
        virtual void            Reset() { m_earlyOutFraction = TraitsType::kInitialEarlyOutFraction; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : When running a query through the NarrowPhase, this will be called for every body that is
        ///     potentially colliding. It allows collecting additional information need by the collision collector
        ///     implementation from the body under lock protection before AddHit() is called.
        ///     (e.g. the user data pointer or the velocity of the Body).
        //----------------------------------------------------------------------------------------------------
        virtual void            OnBody([[maybe_unused]] const Body& body) { /* Collects nothing by default. */ }

        //----------------------------------------------------------------------------------------------------
        /// @brief : When running a query through the NarrowPhase, this will be called after all AddHit() calls have
        ///     been made for a particular Body.
        //----------------------------------------------------------------------------------------------------
        virtual void            OnBodyEnd()                                     { /* Does nothing by default */ }
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set by the collision detection functions to the current TransformedShape that we're colliding
        ///     against before calling AddHit().
        /// @note : Only valid during AddHit()! For performance reasons, the pointer is not reset after leaving
        ///     AddHit() so the Context may point to freed memory.
        //----------------------------------------------------------------------------------------------------
        void                    SetContext(const TransformedShape* pContext)   { m_pContext = pContext; }
        const TransformedShape* GetContext() const                             { return m_pContext; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : This function can be used to set some user data on the collision collector.
        //----------------------------------------------------------------------------------------------------
        virtual void            SetUserData([[maybe_unused]] const uint64_t& data) { /* Does nothing by default */ }

        //----------------------------------------------------------------------------------------------------
        /// @brief : This function will be called for every hit found; it's up to the application to decide how
        ///     to store the hit result.
        //----------------------------------------------------------------------------------------------------
        virtual void            AddHit(const ResultType& result) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Update the early out fraction (should get lower than the stored value). 
        //----------------------------------------------------------------------------------------------------
        void                    UpdateEarlyOutFraction(const float fraction) { NES_ASSERT(fraction <= m_earlyOutFraction);  m_earlyOutFraction = fraction; };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the early out fraction to a specified value.
        //----------------------------------------------------------------------------------------------------
        void                    ResetEarlyOutFraction(const float fraction = TraitsType::kInitialEarlyOutFraction) { m_earlyOutFraction = fraction; };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Force the collision detection algorithm to terminate as soon as possible. Call this from
        ///     AddHit() when a satisfying hit is found.
        //----------------------------------------------------------------------------------------------------
        void                    ForceEarlyOut() { m_earlyOutFraction = TraitsType::kShouldEarlyOutFraction; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : When true, the collector will no longer accept any additional hits and the collision
        ///     detection routine should early out as soon as possible.
        //----------------------------------------------------------------------------------------------------
        bool                    ShouldEarlyOut() const { return m_earlyOutFraction <= TraitsType::kShouldEarlyOutFraction; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current early out value. 
        //----------------------------------------------------------------------------------------------------
        float                   GetEarlyOutFraction() const { return m_earlyOutFraction; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current early out value but make sure it's bigger than zero. This is used for
        ///     shape casting as negative values are used for penetration.
        //----------------------------------------------------------------------------------------------------
        float                   GetPositiveEarlyOutFraction() const { return std::max(FLT_MIN, m_earlyOutFraction); }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Collector implementation that collects all hits and can optionally sort them by distance. 
    ///	@tparam CollectorType : Type of Collector that this class inherits from.
    //----------------------------------------------------------------------------------------------------
    template <typename CollectorType>
    class AllHitCollisionCollector : public CollectorType
    {
    public:
        using ResultType = typename CollectorType::ResultType;

        virtual void Reset() override
        {
            CollectorType::Reset();
            m_hits.clear();
        }

        virtual void AddHit(const ResultType& result) override
        {
            m_hits.push_back(result);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sorts all collected hits from closest to furthest. 
        //----------------------------------------------------------------------------------------------------
        void Sort()
        {
            QuickSort(m_hits.begin(), m_hits.end(), [](const ResultType& left, const ResultType& right)
            {
                return left.GetEarlyOutFraction() < right.GetEarlyOutFraction();
            });
        }

        bool HadHit() const
        {
            return !m_hits.empty();
        }
        
        std::vector<ResultType> m_hits;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Collector implementation that collects the closest/deepest hit. 
    ///	@tparam CollectorType : Type of Collector that this class inherits from.
    //----------------------------------------------------------------------------------------------------
    template <typename CollectorType>
    class ClosestHitCollisionCollector : public CollectorType
    {
    public:
        using ResultType = typename CollectorType::ResultType;

        /// The closest Hit Result:
        ResultType m_hit;
        
    private:
        bool m_hadHit = false;
        
    public:
        virtual void Reset() override
        {
            CollectorType::Reset();
            m_hadHit = false;
        }

        virtual void AddHit(const ResultType& result) override
        {
            const float earlyOut = result.GetEarlyOutFraction();
            if (!m_hadHit || earlyOut < m_hit.GetEarlyOutFraction())
            {
                CollectorType::UpdateEarlyOutFraction(earlyOut);

                // Store the new hit
                m_hit = result;
                m_hadHit = true;
            }
        }

        bool HadHit() const
        {
            return m_hadHit;
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Collector implementation that collects the closest hit for each body and optionally sorts
    ///     them from closest to furthest.
    ///	@tparam CollectorType : Type of Collector that we are inheriting from.
    //----------------------------------------------------------------------------------------------------
    template <typename CollectorType>
    class ClosestHitPerBodyCollisionCollector : public CollectorType
    {
    public:
        using ResultType = typename CollectorType::ResultType;
        std::vector<ResultType> m_hits;

    private:
        float m_previousEarlyOutFraction = std::numeric_limits<float>::min();
        bool m_hadHit = false;

    public:
        virtual void Reset() override
        {
            CollectorType::Reset();
            
            m_hits.clear();
            m_hadHit = false;
        }

        virtual void OnBody(const Body& body) override
        {
            // Store the early out fraction so we can restore it after we've collected all
            // hits for the Body.
            m_previousEarlyOutFraction = CollectorType::GetEarlyOutFraction();
        }

        virtual void AddHit(const ResultType& result) override
        {
            const float earlyOut = result.GetEarlyOutFraction();
            if (!m_hadHit || earlyOut < CollectorType::GetEarlyOutFraction())
            {
                // Update early out fraction to avoid spending work on collecting further hits
                // for this Body
                CollectorType::UpdateEarlyOutFraction(earlyOut);

                if (!m_hadHit)
                {
                    // First time we have a hit we append it to the array.
                    m_hits.push_back(result);
                    m_hadHit = true;
                }

                else
                {
                    // Closer hits will override the previous one.
                    m_hits.back() = result;
                }
            }
        }

        virtual void OnBodyEnd() override
        {
            if (m_hadHit)
            {
                // Reset the early out fraction to the configured value so that we will continue
                // to collect hits at any distance for other bodies:
                NES_ASSERT(m_previousEarlyOutFraction != std::numeric_limits<float>::min());
                CollectorType::ResetEarlyOutFraction(m_previousEarlyOutFraction);
                m_hadHit = false;
            }

            // For asserting purposes, we reset the stored early out fraction so we can detect that
            // OnBody was called.
            NES_IF_LOGGING_ENABLED(m_previousEarlyOutFraction = std::numeric_limits<float>::min());
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sort the hits from closest to furthest. 
        //----------------------------------------------------------------------------------------------------
        void Sort()
        {
            QuickSort(m_hits.begin(), m_hits.end(), [](const ResultType& left, const ResultType& right)
            {
                return left.GetEarlyOutFraction() < right.GetEarlyOutFraction();           
            });
        }

        bool HadHit() const
        {
            return !m_hits.empty();
        }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Collector implementation that collects the first hit that is detected.
    ///	@tparam CollectorType : Type of Collector that we are inheriting from.
    //----------------------------------------------------------------------------------------------------
    template <typename CollectorType>
    class AnyHitCollisionCollector : public CollectorType
    {
    public:
        using ResultType = typename CollectorType::ResultType;
        ResultType m_hit; /// First hit found.
        
    private:
        bool m_hadHit = false;

    public:
        virtual void Reset() override
        {
            CollectorType::Reset();
            m_hadHit = false;
        }

        virtual void AddHit(const ResultType& result) override
        {
            NES_ASSERT(!m_hadHit);

            // Abort any further testing:
            CollectorType::ForceEarlyOut();

            // Store the hit
            m_hit = result;
            m_hadHit = true;
        }

        bool HadHit() const
        {
            return m_hadHit;
        }
    };
}
