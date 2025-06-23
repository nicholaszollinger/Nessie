// LargeIslandSplitter.h
#pragma once
#include "Core/Config.h"
#include "Core/Thread/Atomics.h"

namespace nes
{
    class Body;
    class BodyID;
    class IslandBuilder;
    class StackAllocator;
    class Constraint;
    class BodyManager;
    class ContactConstraintManager;
    class CalculateSolverSteps;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Assigns bodies in large islands to multiple groups that can run in parallel.
    ///
    ///     This basically implements what is described in: High-Performance Physical Simulations on Next-Generation Architecture with Many Cores by Chen et al.
    ///     See: http://web.eecs.umich.edu/~msmelyan/papers/physsim_onmanycore_itj.pdf section "PARALLELIZATION METHODOLOGY"
    //----------------------------------------------------------------------------------------------------
    class LargeIslandSplitter
    {
        using SplitMask = uint32;
        
    public:
        static constexpr uint   kNumSplits = sizeof(SplitMask) * 8; 
        static constexpr uint   kNonParallelSplitIndex = kNumSplits - 1;
        static constexpr uint   kLargeIslandThreshold = 128;             /// If the number of constraints plus contacts in an island is larger than this, we will try to split the island.

        //----------------------------------------------------------------------------------------------------
        /// @brief : Status Code for retrieving a batch.
        //----------------------------------------------------------------------------------------------------
        enum class EStatus
        {
            WaitingForBatch,            /// Work is expected to be available later.
            BatchRetrieved,             /// Work is being returned.
            AllBatchesDone,             /// No further work is expected from this.
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Describes a split of constraints and contacts. 
        //----------------------------------------------------------------------------------------------------
        struct Split
        {
            uint32              m_contactBufferBegin;        /// Beginning of the contact buffer (Offset relative to m_contactAndConstraintIndices).
            uint32              m_contactBufferEnd;          /// End of the contact buffer. 
            uint32              m_constraintBufferBegin;     /// Beginning of the constraint buffer (Offset relative to m_contactAndConstraintIndices).
            uint32              m_constraintBufferEnd;       /// End of the constraint buffer.
            
            inline uint         GetNumContacts() const          { return m_contactBufferEnd - m_contactBufferBegin; }
            inline uint         GetNumConstraints() const       { return m_constraintBufferEnd - m_constraintBufferBegin; }
            inline uint         GetNumItems() const             { return GetNumContacts() + GetNumConstraints(); }
        };

        //----------------------------------------------------------------------------------------------------
        /// @brief : Structure that describes the resulting splits from the large island splitter. 
        //----------------------------------------------------------------------------------------------------
        struct Splits
        {
            enum EIterationStatus : uint64
            {
                StatusIterationMask		= 0xffff000000000000,
                StatusIterationShift	= 48,
                StatusSplitMask			= 0x0000ffff00000000,
                StatusSplitShift		= 32,
                StatusItemMask			= 0x00000000ffffffff,
            };

            Split               m_splits[kNumSplits];       /// Data per split.
            uint32              m_IslandIndex;              /// Index of the island that was split.
            uint                m_numSplits;                /// Number of splits that were created (excluding the non-parallel split).
            int                 m_numIterations;            /// Number of iterations to do.
            int                 m_numVelocitySteps;         /// Number of velocity steps to perform (cached for 2nd sub-step).
            int                 m_numPositionSteps;         /// Number of position steps to perform.
            std::atomic<uint64> m_status;                   /// Status of the split, see EIterationStatus.
            std::atomic<uint>   m_itemsProcessed;           /// Number of items that have been marked as processsed.

            
            inline uint         GetNumSplits() const                    { return m_numSplits; }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the beginning and end index of the constraints in a particular split. 
            //----------------------------------------------------------------------------------------------------
            inline void         GetConstraintsInSplit(const uint splitIndex, uint32& outConstraintsBegin, uint32& outConstraintsEnd) const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the beginning and end index of the contacts in a particular split. 
            //----------------------------------------------------------------------------------------------------
            inline void         GetContactsInSplit(const uint splitIndex, uint32& outContactsBegin, uint32& outContactsEnd) const;

            //----------------------------------------------------------------------------------------------------
            /// @brief : Reset the current status so that no work can be picked up from this split. 
            //----------------------------------------------------------------------------------------------------
            inline void         ResetStatus()                           { m_status.store(StatusItemMask, std::memory_order_relaxed); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Make the first batch available to other threads. 
            //----------------------------------------------------------------------------------------------------
            inline void         StartFirstBatch();

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the next batch to process. 
            //----------------------------------------------------------------------------------------------------
            EStatus             FetchNextBatch(uint32& outConstraintsBegin, uint32& outConstraintsEnd, uint32& outContactsBegin, uint32& outContactsEnd, bool& outFirstIteration);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Mark a batch as processed. 
            //----------------------------------------------------------------------------------------------------
            void                MarkBatchProcessed(const uint numProcessed, bool& outLastIteration, bool& outFinalBatch);

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the iteration number from a status value.
            //----------------------------------------------------------------------------------------------------
            static inline int   GetIteration(const uint64 status)       { return static_cast<int>((status & StatusIterationMask) >> StatusIterationShift); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the split index from a status value. 
            //----------------------------------------------------------------------------------------------------
            static inline uint  GetSplit(const uint64 status)           { return static_cast<uint>((status & StatusSplitMask) >> StatusSplitShift); }

            //----------------------------------------------------------------------------------------------------
            /// @brief : Get the item index from a status value. 
            //----------------------------------------------------------------------------------------------------
            static inline uint  GetItem(const uint64 status)            { return static_cast<uint>(status & StatusItemMask); }
        };

    public:
        LargeIslandSplitter() = default;
        LargeIslandSplitter(const LargeIslandSplitter&) = delete;
        LargeIslandSplitter(LargeIslandSplitter&&) = delete;
        LargeIslandSplitter& operator=(const LargeIslandSplitter&) = delete;
        LargeIslandSplitter& operator=(LargeIslandSplitter&&) noexcept = delete;
        ~LargeIslandSplitter();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Prepare the island splitter by allocating necessary memory.
        //----------------------------------------------------------------------------------------------------
        void                    Prepare(const IslandBuilder& islandBuilder, const uint32 numActiveBodies, StackAllocator* pAllocator);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Assign two bodies to a split. Returns the split index.
        //----------------------------------------------------------------------------------------------------
        uint                    AssignSplit(const Body* pBody1, const Body* pBody2);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Force a body to be in a non-parallel split. Returns the split index. 
        //----------------------------------------------------------------------------------------------------
        uint                    AssignToNonParallelSplit(const Body* pBody);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Splits up an island. The created splits will be added ot the array of batches and can be
        ///     fetched with FetchNextBatch(). Returns false if the island did not need to be split.
        //----------------------------------------------------------------------------------------------------
        bool                    SplitIsland(const uint32 islandIndex, const IslandBuilder& builder, const BodyManager& bodyManager, const ContactConstraintManager& contactManager, Constraint** activeConstraints, CalculateSolverSteps& stepsCalculator);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Fetch the next batch to process, returns a handle in outSplitIslandIndex that must be provided to MarkBatchProcessed when complete
        //----------------------------------------------------------------------------------------------------
        EStatus                 FetchNextBatch(uint& outSplitIslandIndex, uint32*& outConstraintsBegin, uint32*& outConstraintsEnd, uint32*& outContactsBegin, uint32*& outContactsEnd, bool& outFirstIteration);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Mark a batch as processed. Should be called after FetchNextBatch().
        //----------------------------------------------------------------------------------------------------
        void                    MarkBatchProcessed(const uint splitIslandIndex, const uint32* constraintsBegin, const uint32* constraintsEnd, const uint32* contactsBegin, const uint32* contactsEnd, bool& outLastIteration, bool& outFinalBatch);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the island index of the island that was split for a particular split index. 
        //----------------------------------------------------------------------------------------------------
        inline uint32           GetIslandIndex(const uint splitIslandIndex) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Prepare the island splitter for iterating over the split islands again for position solving.
        ///     Marks all batches as startable.
        //----------------------------------------------------------------------------------------------------
        void                    PrepareForSolverPositions();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reset the island splitter, deallocating stack memory. 
        //----------------------------------------------------------------------------------------------------
        void                    Reset(StackAllocator* pAllocator);

    private:
        static constexpr uint   kSplitCombineThreshold = 32;                    /// If the number of constraints plus contacts in a split is lower than this, we will merge this split into the 'non-parallel split'.
        static constexpr uint   kBatchSize = 16;                                /// Number of items to process in a constraint batch.

        uint32                  m_numActiveBodies = 0;                          /// Cached number of active bodies.
        SplitMask*              m_splitMasks = nullptr;                         /// Bits that indicate for each body in the BodyManager::m_activeBodies array which split they already belong to.
        uint32*                 m_contactAndConstraintsSplitIndex = nullptr;    /// Buffer to store the split index per constraint or contact.
        uint32*                 m_contactAndConstraintIndices = nullptr;        /// Buffer to store the ordered constraint indices per split.
        uint                    m_contactAndConstraintsSize = 0;                /// Size of both the m_contactAndConstraintsSplitIndex and m_contactAndConstraintIndices arrays.
        std::atomic<uint>       m_contactAndConstraintsNextFree { 0 };    /// Next element that is free in both buffers.
        uint                    m_numSplitIslands = 0;                          /// Total number of islands that required splitting.
        Splits*                 m_splitIslands = nullptr;                       /// Array of islands that required splitting.
        std::atomic<uint>       m_nextSplitIslandIndex = 0;                     /// Next split island index to pick from m_splitIslands.
    };
}