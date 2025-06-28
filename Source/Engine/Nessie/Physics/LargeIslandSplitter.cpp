// LargeIslandSplitter.cpp
#include "LargeIslandSplitter.h"

#include "IslandBuilder.h"
#include "Constraints/CalculateSolverSteps.h"
#include "Constraints/ContactConstraintManager.h"
#include "Body/BodyManager.h"
#include "Constraints/Constraint.h"
#include "Core/Memory/StackAllocator.h"

namespace nes
{
    void LargeIslandSplitter::Splits::GetConstraintsInSplit(const uint splitIndex, uint32& outConstraintsBegin, uint32& outConstraintsEnd) const
    {
        NES_ASSERT(splitIndex < kNumSplits);
        const Split& split = m_splits[splitIndex];
        outConstraintsBegin = split.m_constraintBufferBegin;
        outConstraintsEnd = split.m_constraintBufferEnd;
    }

    void LargeIslandSplitter::Splits::GetContactsInSplit(const uint splitIndex, uint32& outContactsBegin, uint32& outContactsEnd) const
    {
        NES_ASSERT(splitIndex < kNumSplits);
        const Split& split = m_splits[splitIndex];
        outContactsBegin = split.m_contactBufferBegin;
        outContactsEnd = split.m_contactBufferEnd;
    }

    void LargeIslandSplitter::Splits::StartFirstBatch()
    {
        const uint splitIndex = m_numSplits > 0? 0 : kNonParallelSplitIndex;
        m_status.store(static_cast<uint64>(splitIndex) << StatusSplitShift, std::memory_order_release);
    }

    LargeIslandSplitter::EStatus LargeIslandSplitter::Splits::FetchNextBatch(uint32& outConstraintsBegin, uint32& outConstraintsEnd, uint32& outContactsBegin, uint32& outContactsEnd, bool& outFirstIteration)
    {
        // First check if we can get a new batch (doing a read to avoid hammering an atomic with an atomic subtraction).
        // Note this also avoids overflowing the status counter if we're done, but there is still one thread processing the items.
        {
            const uint64 status = m_status.load(std::memory_order_acquire);

            // Check for special value that indicates that the splits are still being built.
            // Note: We do not check for this condition again below as we reset all splits before kicking off jobs that fetch batches of work.
            if (status == StatusItemMask)
                return EStatus::WaitingForBatch;

            // Next check if all items have been processed. Note: We do this after checking if the job can be started as m_numIterations is
            // not initialized until the split is started.
            if (GetIteration(status) >= m_numIterations)
                return EStatus::AllBatchesDone;

            const uint item = GetItem(status);
            const uint splitIndex = GetSplit(status);
            if (splitIndex == kNonParallelSplitIndex)
            {
                // Non-parallel split needs to be taken as a single batch; only the thread that takes element 0 will do it.
                if (item != 0)
                    return EStatus::WaitingForBatch;
            }
            else
            {
                // Parallel split is split into batches.
                NES_ASSERT(splitIndex < m_numSplits);
                const Split& split = m_splits[splitIndex];
                if (item >= split.GetNumItems())
                    return EStatus::WaitingForBatch;
            }
        }

        // The try to get the actual batch
        const uint64 status = m_status.fetch_add(kBatchSize, std::memory_order_acquire);
        const int iteration = GetIteration(status);
        if (iteration >= m_numIterations)
            return EStatus::AllBatchesDone;

        const uint splitIndex = GetSplit(status);
        NES_ASSERT(splitIndex < m_numSplits || splitIndex ==  kNonParallelSplitIndex);
        const Split& split = m_splits[splitIndex];
        const uint itemBegin = GetItem(status);
        if (splitIndex == kNonParallelSplitIndex)
        {
            if (itemBegin == 0)
            {
                // Non-parallel split always goes as a single batch
                outConstraintsBegin = split.m_constraintBufferBegin;
                outConstraintsEnd = split.m_constraintBufferEnd;
                outContactsBegin = split.m_contactBufferBegin;
                outContactsEnd = split.m_contactBufferEnd;
                outFirstIteration = iteration == 0;
                return EStatus::BatchRetrieved;
            }
            
            // Otherwise, we are done with this split.
            return EStatus::WaitingForBatch;
        }

        // Parallel split is split into batches.
        const uint numConstraints = split.GetNumConstraints();
        const uint numContacts = split.GetNumContacts();
        const uint numItems = numConstraints + numContacts;
        if (itemBegin >= numItems)
            return EStatus::WaitingForBatch;

        const uint itemEnd = math::Min(itemBegin + kBatchSize, numItems);
        if (itemEnd >= numConstraints)
        {
            if (itemBegin < numConstraints)
            {
                // Partially from constraints and partially from contacts.
                outConstraintsBegin = split.m_constraintBufferBegin + itemBegin;
                outConstraintsEnd = split.m_constraintBufferEnd;
            }
            else
            {
                // Only Contacts
                outConstraintsBegin = 0;
                outConstraintsEnd = 0;
            }

            outContactsBegin = split.m_contactBufferBegin + (math::Max(itemBegin, numConstraints) - numConstraints);
            outContactsEnd = split.m_constraintBufferEnd + (itemEnd - numConstraints);
        }
        else
        {
            // Only constraints
            outConstraintsBegin = split.m_constraintBufferBegin + itemBegin;
            outConstraintsEnd = split.m_constraintBufferEnd + itemEnd;

            outContactsBegin = 0;
            outContactsEnd = 0;
        }

        outFirstIteration = iteration == 0;
        return EStatus::BatchRetrieved;
    }

    void LargeIslandSplitter::Splits::MarkBatchProcessed(const uint numProcessed, bool& outLastIteration, bool& outFinalBatch)
    {
        // We fetched this batch; nobody should change the split and/or iteration until we mark the last batch as processed so we can safely get the current status.
        const uint64 status = m_status.load(std::memory_order_relaxed);
        uint splitIndex = GetSplit(status);
        NES_ASSERT(splitIndex < m_numSplits || splitIndex ==  kNonParallelSplitIndex);
        const Split& split = m_splits[splitIndex];
        const uint numItemsInSplit = split.GetNumItems();

        // Determine if this is the last iteration before possible incrementing it.
        int iteration = GetIteration(status);
        outLastIteration = iteration == m_numIterations - 1;

        // Add the number of items we processed to the total number of items processed.
        // Note: This needs to happen after we read the status as the other threads may update the status after we mark items as processed.
        const uint totalItemsProcessed = m_itemsProcessed.fetch_add(numProcessed, std::memory_order_acq_rel) + numProcessed;

        // Check if we're at the end of the split:
        if (totalItemsProcessed >= numItemsInSplit)
        {
            NES_ASSERT(totalItemsProcessed == numItemsInSplit); // Should not overflow; that means we're retiring more items than we should process.

            // Set items processed back to 0 for the next split/iteration.
            m_itemsProcessed.store(0, std::memory_order_release);

            // Determine the next split
            do
            {
                if (splitIndex == kNonParallelSplitIndex)
                {
                    // At start of next iteration
                    splitIndex = 0;
                    ++iteration;
                }
                else
                {
                    // At the start of the next split
                    ++splitIndex;
                }

                // If we're beyond the end of splits, go to the non-parallel split.
                if (splitIndex >= m_numSplits)
                    splitIndex = kNonParallelSplitIndex;
                
            } while (iteration < m_numIterations && m_splits[splitIndex].GetNumItems() == 0); // We don't support processing empty splits. Skip to the next in this case.

            // Set our status to the new split index and iteration number.
            m_status.store(static_cast<uint64>(iteration) << StatusIterationShift | static_cast<uint64>(splitIndex) << StatusSplitShift, std::memory_order_release);
        }

        // Track if this is the final batch
        outFinalBatch = iteration >= m_numIterations;
    }

    LargeIslandSplitter::~LargeIslandSplitter()
    {
        // Ensure that all memory was cleaned up.
        NES_ASSERT(m_splitMasks == nullptr);
        NES_ASSERT(m_contactAndConstraintsSplitIndex == nullptr);
        NES_ASSERT(m_contactAndConstraintIndices == nullptr);
        NES_ASSERT(m_splitIslands == nullptr);
    }

    void LargeIslandSplitter::Prepare(const IslandBuilder& islandBuilder, const uint32 numActiveBodies, StackAllocator* pAllocator)
    {
        // Count the total number of constraints and contacts that we will be putting in splits.
        m_contactAndConstraintsSize = 0;
        for (uint32 island = 0; island < islandBuilder.GetNumIslands(); ++island)
        {
            // Get the contacts in this island
            uint32* contactsBegin;
            uint32* contactsEnd;
            islandBuilder.GetContactsInIsland(island, contactsBegin, contactsEnd);
            const uint numContactsInIsland = static_cast<uint>(contactsEnd - contactsBegin);

            // Get the constraints in this island
            uint32* constraintsBegin;
            uint32* constraintsEnd;
            islandBuilder.GetConstraintsInIsland(island, constraintsBegin, constraintsEnd);
            const uint numConstraintsInIsland = static_cast<uint>(constraintsEnd - constraintsBegin);

            const uint islandSize = numContactsInIsland + numConstraintsInIsland;

            // This island needs to be split.
            if (islandSize >= kLargeIslandThreshold)
            {
                ++m_numSplitIslands;
                m_contactAndConstraintsSize += islandSize;
            }

            else
            {
                // If the island does not meet the split threshold, we can break because
                // the Island builder sorts the islands from largest to smallest!
                break;
            }
        }

        if (m_contactAndConstraintsSize > 0)
        {
            m_numActiveBodies = numActiveBodies;

            // Allocate split mask buffer
            m_splitMasks = static_cast<SplitMask*>(pAllocator->Allocate(m_numActiveBodies * sizeof(SplitMask)));

            // Allocate contact and constraint buffers
            const uint contactAndConstraintsIndicesSize = m_contactAndConstraintsSize * sizeof(uint32);
            m_contactAndConstraintsSplitIndex = static_cast<uint32*>(pAllocator->Allocate(contactAndConstraintsIndicesSize));
            m_contactAndConstraintIndices = static_cast<uint32*>(pAllocator->Allocate(contactAndConstraintsIndicesSize));

            // Allocate island split buffer
            m_splitIslands = static_cast<Splits*>(pAllocator->Allocate(m_numSplitIslands * sizeof(Splits)));

            // Prevent any of the splits from begin picked up as work
            for (uint i = 0; i < m_numSplitIslands; ++i)
            {
                m_splitIslands[i].ResetStatus();
            }
        }
    }

    uint LargeIslandSplitter::AssignSplit(const Body* pBody1, const Body* pBody2)
    {
        const uint32 index1 = pBody1->Internal_GetIndexInActiveBodies();
        const uint32 index2 = pBody2->Internal_GetIndexInActiveBodies();

        // Test if either index is not active or dynamic.
        if (index1 == Body::kInactiveIndex || !pBody1->IsDynamic())
        {
            // Body 1 is not active or a kinematic body, so we only need to set 1 body
            NES_ASSERT(index2 < m_numActiveBodies);
            SplitMask& mask = m_splitMasks[index2];
            const uint split = math::Min(math::CountTrailingZeros(~mask), kNonParallelSplitIndex);
            mask |= static_cast<SplitMask>(1U << split);
            return split;
        }

        else if (index2 == Body::kInactiveIndex || !pBody2->IsDynamic())
        {
            // Body 1 is not active or a kinematic body, so we only need to set 1 body
            NES_ASSERT(index1 < m_numActiveBodies);
            SplitMask& mask = m_splitMasks[index1];
            const uint split = math::Min(math::CountTrailingZeros(~mask), kNonParallelSplitIndex);
            mask |= static_cast<SplitMask>(1U << split);
            return split;
        }

        else
        {
            // If both bodies are active, we need to set 2 bodies.
            NES_ASSERT(index1 < m_numActiveBodies);
            NES_ASSERT(index2 < m_numActiveBodies);
            SplitMask& mask1 = m_splitMasks[index1];
            SplitMask& mask2 = m_splitMasks[index2];
            const uint split = math::Min(math::CountTrailingZeros((~mask1) & (~mask2)), kNonParallelSplitIndex);
            const SplitMask mask = (1U << split);
            mask1 |= mask;
            mask2 |= mask;
            return split;
        }
    }

    uint LargeIslandSplitter::AssignToNonParallelSplit(const Body* pBody)
    {
        const uint32 index = pBody->Internal_GetIndexInActiveBodies();
        if (index != Body::kInactiveIndex)
        {
            NES_ASSERT(index < m_numActiveBodies);
            m_splitMasks[index] |= 1U << kNonParallelSplitIndex;
        }

        return kNonParallelSplitIndex;
    }

    bool LargeIslandSplitter::SplitIsland(const uint32 islandIndex, const IslandBuilder& builder, const BodyManager& bodyManager, const ContactConstraintManager& contactManager, Constraint** activeConstraints, CalculateSolverSteps& stepsCalculator)
    {
        // Get the contacts in the island.
        uint32* contactsBegin;
        uint32* contactsEnd;
        builder.GetContactsInIsland(islandIndex, contactsBegin, contactsEnd);
        const uint numContactsInIsland = static_cast<uint>(contactsEnd - contactsBegin);
        
        // Get the constraints in the island.
        uint32* constraintsBegin;
        uint32* constraintsEnd;
        builder.GetConstraintsInIsland(islandIndex, constraintsBegin, constraintsEnd);
        const uint numConstraintsInIsland = static_cast<uint>(constraintsEnd - constraintsBegin);
        
        // Check if it exceeds the threshold:
        const uint islandSize = numContactsInIsland + numConstraintsInIsland;
        if (islandSize < kLargeIslandThreshold)
            return false;

        // Get the bodies in the island.
        BodyID* bodiesBegin;
        BodyID* bodiesEnd;
        builder.GetBodiesInIsland(islandIndex, bodiesBegin, bodiesEnd);

        // Reset the split mask for all bodies in this island.
        Body const* const* pBodies = bodyManager.GetBodies().data();
        for (const BodyID* pBody = bodiesBegin; pBody < bodiesEnd; ++pBody)
        {
            m_splitMasks[pBodies[pBody->GetIndex()]->Internal_GetIndexInActiveBodies()] = 0;
        }

        // Count the number of contacts and constraints per split
        uint numContactsInSplit[kNumSplits] = {};
        uint numConstraintsInSplit[kNumSplits] = {};

        // Get space to store split indices
        uint offset = m_contactAndConstraintsNextFree.fetch_add(islandSize, std::memory_order_relaxed);
        uint32* contactSplitIndex = m_contactAndConstraintsSplitIndex + offset;
        uint32* constraintsSplitIndex = contactSplitIndex + numContactsInIsland;

        // Assign the contacts to a split
        uint32* currentContactSplitIndex = contactSplitIndex;
        for (const uint32* pCurrent = contactsBegin; pCurrent < contactsEnd; ++pCurrent)
        {
            const Body* pBody1;
            const Body* pBody2;
            contactManager.GetAffectedBodies(*pCurrent, pBody1, pBody2);
            uint split = AssignSplit(pBody1, pBody2);
            numContactsInSplit[split]++;
            *currentContactSplitIndex++ = split;

            if (pBody1->IsDynamic())
                stepsCalculator(pBody1->GetMotionPropertiesUnchecked());
            if (pBody2->IsDynamic())
                stepsCalculator(pBody2->GetMotionPropertiesUnchecked());
        }

        // Assign the constraints to a split
        uint32* currentConstraintSplitIndex = constraintsSplitIndex;
        for (const uint32* pCurrent = constraintsBegin; pCurrent < constraintsEnd; ++pCurrent)
        {
            const Constraint* pConstraint = activeConstraints[*pCurrent];
            uint split = pConstraint->BuildIslandSplits(*this);
            numConstraintsInSplit[split]++;
            *currentConstraintSplitIndex++ = split;

            stepsCalculator(pConstraint);
        }

        stepsCalculator.Finalize();

        // Start with 0 splits
        uint splitRemapTable[kNumSplits];
        uint newSplitIndex = m_nextSplitIslandIndex.fetch_add(1, std::memory_order_relaxed);
        NES_ASSERT(newSplitIndex < m_numSplitIslands);
        Splits& splits = m_splitIslands[newSplitIndex];
        splits.m_IslandIndex = islandIndex;
        splits.m_numSplits = 0;
        splits.m_numIterations = stepsCalculator.GetNumVelocitySteps() + 1; // Iteration 0 is used for warm starting.
        splits.m_numVelocitySteps = stepsCalculator.GetNumVelocitySteps();
        splits.m_numPositionSteps = stepsCalculator.GetNumPositionSteps();
        splits.m_itemsProcessed.store(0, std::memory_order_release);

        // Allocate space to store the sorted constraint and contact indices per split
        uint32* contactBufferCurrent[kNumSplits];
        uint32* constraintBufferCurrent[kNumSplits];
        for (uint splitIndex = 0; splitIndex < kNumSplits; ++splitIndex)
        {
            // If this split doesn't contain enough constraints and contacts, we will combine is with the non-parallel split.
            if (numConstraintsInSplit[splitIndex] + numContactsInSplit[splitIndex] < kSplitCombineThreshold
                && splitIndex < kNonParallelSplitIndex) // The non-parallel split cannot merge into itself.
            {
                // Remap it
                splitRemapTable[splitIndex] = kNonParallelSplitIndex;

                // Add the counts to the non-parallel split
                numContactsInSplit[kNonParallelSplitIndex] += numContactsInSplit[splitIndex];
                numConstraintsInSplit[kNonParallelSplitIndex] += numConstraintsInSplit[splitIndex];
            }
            else
            {
                // This split is valid, map it to the next empty slot.
                uint targetSplit;
                if (splitIndex < kNonParallelSplitIndex)
                    targetSplit = splits.m_numSplits++;
                else
                    targetSplit = kNonParallelSplitIndex;

                Split& split = splits.m_splits[targetSplit];
                splitRemapTable[splitIndex] = targetSplit;

                // Allocate space for contacts
                split.m_contactBufferBegin = offset;
                split.m_contactBufferEnd = split.m_contactBufferBegin + numContactsInSplit[splitIndex];

                // Allocate space for constraints
                split.m_constraintBufferBegin = split.m_contactBufferEnd;
                split.m_constraintBufferEnd = split.m_constraintBufferBegin + numConstraintsInSplit[splitIndex];

                // Store start for each split
                contactBufferCurrent[targetSplit] = m_contactAndConstraintIndices + split.m_contactBufferBegin;
                constraintBufferCurrent[targetSplit] = m_contactAndConstraintIndices + split.m_constraintBufferBegin;

                // Update offset
                offset = split.m_constraintBufferEnd;
            }
        }

        // Split the contacts
        for (uint c = 0; c < numContactsInIsland; ++c)
        {
            const uint split = splitRemapTable[contactSplitIndex[c]];
            *contactBufferCurrent[split]++ = contactsBegin[c];
        }

        // Split the constraints
        for (uint c = 0; c < numConstraintsInIsland; ++c)
        {
            const uint split = splitRemapTable[constraintsSplitIndex[c]];
            *constraintBufferCurrent[split]++ = constraintsBegin[c];
        }

    #ifdef NES_ASSERTS_ENABLED
        for (uint s = 0; s < kNumSplits; ++s)
        {
            // If there are no more splits, process the non-parallel split
            if (s >= splits.m_numSplits)
                s = kNonParallelSplitIndex;

            // Check that we wrote all elements
            Split& split = splits.m_splits[s];
            NES_ASSERT(contactBufferCurrent[s] == m_contactAndConstraintIndices + split.m_contactBufferEnd);
            NES_ASSERT(constraintBufferCurrent[s] == m_contactAndConstraintIndices + split.m_constraintBufferEnd);
        }
        
        #ifdef NES_DEBUG
        // Validate that the splits are indeed not touching the same body
        for (uint s = 0; s < splits.m_numSplits; ++s)
        {
            std::vector<bool> bodyUsed(m_numActiveBodies, false);

            // Validate contacts
            uint32 splitContactsBegin;
            uint32 splitContactsEnd;
            splits.GetContactsInSplit(s, splitContactsBegin, splitContactsEnd);
            for (uint32* c = m_contactAndConstraintIndices + splitContactsBegin; c < m_contactAndConstraintIndices + splitContactsEnd; ++c)
            {
                const Body* pBody1;
                const Body* pBody2;
                contactManager.GetAffectedBodies(*c, pBody1, pBody2);

                const uint32 index1 = pBody1->Internal_GetIndexInActiveBodies();
                if (index1 != Body::kInactiveIndex && pBody1->IsDynamic())
                {
                    NES_ASSERT(!bodyUsed[index1]);
                    bodyUsed[index1] = true;
                }

                const uint32 index2 = pBody2->Internal_GetIndexInActiveBodies();
                if (index2 != Body::kInactiveIndex && pBody2->IsDynamic())
                {
                    NES_ASSERT(!bodyUsed[index2]);
                    bodyUsed[index2] = true;
                }
            }
        }
        #endif
    #endif

        // Allow other threads to pick up this split island now
        splits.StartFirstBatch();
        return true;
    }

    LargeIslandSplitter::EStatus LargeIslandSplitter::FetchNextBatch(uint& outSplitIslandIndex, uint32*& outConstraintsBegin, uint32*& outConstraintsEnd, uint32*& outContactsBegin, uint32*& outContactsEnd, bool& outFirstIteration)
    {
        // We can't be done when all islands haven't been submitted yet.
        const uint numSplitsCreated = m_nextSplitIslandIndex.load(std::memory_order::acquire);
        bool allDone = numSplitsCreated == m_numSplitIslands;

        // Loop over all split islands to find work
        uint32 constraintsBegin;
        uint32 constraintsEnd;
        uint32 contactsBegin;
        uint32 contactsEnd;
        for (Splits* pSplits = m_splitIslands; pSplits < m_splitIslands + numSplitsCreated; ++pSplits)
        {
            switch (pSplits->FetchNextBatch(constraintsBegin, constraintsEnd, contactsBegin, contactsEnd, outFirstIteration))
            {
                case EStatus::AllBatchesDone: break;
                    
                case EStatus::WaitingForBatch:
                {
                    allDone = false;
                    break;
                }
                    
                case EStatus::BatchRetrieved:
                {
                    outSplitIslandIndex = static_cast<uint>(pSplits - m_splitIslands);
                    outConstraintsBegin = m_contactAndConstraintIndices + constraintsBegin;
                    outConstraintsEnd = m_contactAndConstraintIndices + constraintsEnd;
                    outContactsBegin = m_contactAndConstraintIndices + contactsBegin;
                    outContactsEnd = m_contactAndConstraintIndices + contactsEnd;
                    return EStatus::BatchRetrieved;
                }
            }
        }

        return allDone? EStatus::AllBatchesDone : EStatus::WaitingForBatch;
    }

    void LargeIslandSplitter::MarkBatchProcessed(const uint splitIslandIndex, const uint32* constraintsBegin, const uint32* constraintsEnd, const uint32* contactsBegin, const uint32* contactsEnd, bool& outLastIteration, bool& outFinalBatch)
    {
        const uint numItemsProcessed = static_cast<uint>(constraintsEnd - constraintsBegin) + static_cast<uint>(contactsEnd - contactsBegin);

        NES_ASSERT(splitIslandIndex < m_nextSplitIslandIndex.load(std::memory_order_relaxed));
        Splits& splits = m_splitIslands[splitIslandIndex];
        splits.MarkBatchProcessed(numItemsProcessed, outLastIteration, outFinalBatch);
    }

    void LargeIslandSplitter::PrepareForSolverPositions()
    {
        for (Splits* pSplits = m_splitIslands, *pSplitsEnd = m_splitIslands + m_numSplitIslands; pSplits < pSplitsEnd; ++pSplits)
        {
            // Set the number of iterations to the number of position steps.
            pSplits->m_numIterations = pSplits->m_numPositionSteps;

            // We can start again from the first batch
            pSplits->StartFirstBatch();
        }
    }

    void LargeIslandSplitter::Reset(StackAllocator* pAllocator)
    {
        // Everything should have been used.
        NES_ASSERT(m_contactAndConstraintsNextFree.load(std::memory_order_relaxed) == m_contactAndConstraintsSize);
        NES_ASSERT(m_nextSplitIslandIndex.load(std::memory_order_relaxed) == m_numSplitIslands);

        // Free split islands
        if (m_numSplitIslands > 0)
        {
            pAllocator->Free(m_splitIslands, m_numSplitIslands * sizeof(Splits));
            m_splitIslands = nullptr;

            m_numSplitIslands = 0;
            m_nextSplitIslandIndex.store(0, std::memory_order_relaxed);
        }

        // Free contact and constraint buffers
        if (m_contactAndConstraintsSize > 0)
        {
            pAllocator->Free(m_contactAndConstraintIndices, m_contactAndConstraintsSize * sizeof(uint32));
            m_contactAndConstraintIndices = nullptr;

            pAllocator->Free(m_contactAndConstraintsSplitIndex, m_contactAndConstraintsSize * sizeof(uint32));
            m_contactAndConstraintsSplitIndex = nullptr;

            m_contactAndConstraintsSize = 0;
            m_contactAndConstraintsNextFree.store(0, std::memory_order_relaxed);
        }

        // Free split masks
        if (m_splitMasks != nullptr)
        {
            pAllocator->Free(m_splitMasks, m_numActiveBodies * sizeof(SplitMask));
            m_splitMasks = nullptr;
            
            m_numActiveBodies = 0;
        }
    }
}
