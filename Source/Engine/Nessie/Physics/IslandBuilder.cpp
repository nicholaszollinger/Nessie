// IslandBuilder.cpp
#include "IslandBuilder.h"

#include "Body/Body.h"
#include "Nessie/Core/Memory/Memory.h"
#include "Nessie/Core/Memory/StackAllocator.h"
#include "Nessie/Math/Generic.h"

namespace nes
{
    IslandBuilder::~IslandBuilder()
    {
        NES_ASSERT(m_constraintLinks == nullptr);
        NES_ASSERT(m_contactLinks == nullptr);
        NES_ASSERT(m_bodyIslands == nullptr);
        NES_ASSERT(m_bodyIslandEnds == nullptr);
        NES_ASSERT(m_constraintIslands == nullptr);
        NES_ASSERT(m_constraintIslandEnds == nullptr);
        NES_ASSERT(m_contactIslands == nullptr);
        NES_ASSERT(m_contactIslandEnds == nullptr);
        NES_ASSERT(m_islandsSorted == nullptr);
        
        NES_DELETE_ARRAY(m_bodyLinks);
    }

    void IslandBuilder::Init(const uint32 maxActiveBodies)
    {
        m_maxActiveBodies = maxActiveBodies;

        // Link each body to itself, BuildBodyIslands() will restore this so that we don't need to do this each step.
        NES_ASSERT(m_bodyLinks == nullptr);
        m_bodyLinks = NES_NEW_ARRAY(BodyLink, m_maxActiveBodies);
        for (uint32 i = 0; i < m_maxActiveBodies; ++i)
        {
            m_bodyLinks[i].m_linkedTo.store(i, std::memory_order_relaxed);
        }
    }

    void IslandBuilder::PrepareContactConstraints(const uint32 maxContacts, StackAllocator* pAllocator)
    {
        // Must call Init first.
        NES_ASSERT(m_bodyLinks != nullptr);

        // Check that the builder has been reset.
        NES_ASSERT(m_numContacts == 0);
        NES_ASSERT(m_numIslands == 0);

        // Create contact link buffer, not initialized so each contact needs to be explicitly set.
        NES_ASSERT(m_contactLinks == nullptr);
        m_contactLinks = static_cast<uint32*>(pAllocator->Allocate(maxContacts * sizeof(uint32)));
        m_maxContacts = maxContacts;
    }

    void IslandBuilder::PrepareNonContactConstraints(const uint32 numConstraints, StackAllocator* pAllocator)
    {
        // Must call Init first.
        NES_ASSERT(m_bodyLinks != nullptr);
        
        // Check that the builder has been reset.
        NES_ASSERT(m_numIslands == 0);

        // Store number of constraints
        m_numConstraints = numConstraints;

        // Create the constraint link buffer, not initialized so each constraint needs to be explicitly set.
        NES_ASSERT(m_constraintLinks == nullptr);
        m_constraintLinks = static_cast<uint32*>(pAllocator->Allocate(numConstraints * sizeof(uint32)));
    }

    void IslandBuilder::LinkBodies(const uint32 first, const uint32 second)
    {
        // Both bodies need to be active, we don't want to create an island with static objects.
        if (first >= m_maxActiveBodies || second >= m_maxActiveBodies)
            return;

        // Start the algorithm with the two bodies.
        uint32 firstLinkTo = first;
        uint32 secondLinkTo = second;

        for (;;)
        {
            // Follow the chain until we get to the body with the lowest index.
            // If the swap-compare below fails, we'll keep searching form the lowerst index for the new lowest index.
            firstLinkTo = GetLowestBodyIndex(firstLinkTo);
            secondLinkTo = GetLowestBodyIndex(secondLinkTo);

            // If the targets are the same, the bodies are already connected.
            if (firstLinkTo != secondLinkTo)
            {
                // We always link the highest to the lowest.
                if (firstLinkTo < secondLinkTo)
                {
                    // Attempt to link the second to the first.
                    // Since we found this body to be at the end of the chain, it must point to itself, and if it
                    // doesn't, it has been reparented, and we need to retry the algorithm.
                    if (!m_bodyLinks[secondLinkTo].m_linkedTo.compare_exchange_weak(secondLinkTo, firstLinkTo, std::memory_order_relaxed))
                        continue;
                }

                else
                {
                    // Attempt to link the first to the second.
                    // Since we found this body to be at the end of the chain, it must point to itself, and if it
                    // doesn't, it has been reparented, and we need to retry the algorithm.
                    if (!m_bodyLinks[firstLinkTo].m_linkedTo.compare_exchange_weak(firstLinkTo, secondLinkTo, std::memory_order_relaxed))
                        continue;
                }

                // Linking succeeded!
                // Chains of bodies can become really long, resulting in an O(N) loop to find the lowest body index.
                // To prevent this, we attempt to update the link of the bodies that were passed in to directly point
                // to the lowest index that we found. If the value became lower than our lowest link, some other
                // thread must have relinked these bodies in the meantime, so we won't update the value.
                const uint32 lowestLinkTo = math::Min(firstLinkTo, secondLinkTo);
                AtomicMin(m_bodyLinks[first].m_linkedTo, lowestLinkTo, std::memory_order_relaxed);
                AtomicMin(m_bodyLinks[second].m_linkedTo, lowestLinkTo, std::memory_order_relaxed);
                break;
            }
        }
    }

    void IslandBuilder::LinkConstraint(const uint32 constraintIndex, const uint32 first, const uint32 second)
    {
        LinkBodies(first, second);

        NES_ASSERT(constraintIndex < m_numConstraints);
        const uint32 minValue = math::Min(first, second);   // Use the fact that invalid index is 0xfffffff; we want the active body between the two.
        NES_ASSERT(minValue != Body::kInactiveIndex);           // At least one of the bodies must be active.
        m_constraintLinks[constraintIndex] = minValue;
    }

    void IslandBuilder::LinkContact(const uint32 contactIndex, const uint32 first, const uint32 second)
    {
        NES_ASSERT(contactIndex < m_maxContacts);
        m_contactLinks[contactIndex] = math::Min(first, second); // Use the fact that invalid index is 0xfffffff; we want the active body between the two.
    }

    void IslandBuilder::Finalize(const BodyID* pActiveBodies, uint32 numActiveBodies, uint32 numContacts, StackAllocator* pAllocator)
    {
        m_numContacts = numContacts;

        BuildBodyIslands(pActiveBodies, numActiveBodies, pAllocator);
        BuildConstraintIslands(m_constraintLinks, m_numConstraints, m_constraintIslands, m_constraintIslandEnds, pAllocator);
        BuildConstraintIslands(m_contactLinks, m_numContacts, m_contactIslands, m_contactIslandEnds, pAllocator);
        SortIslands(pAllocator);

        m_numPositionSteps = static_cast<uint8*>(pAllocator->Allocate(m_numIslands * sizeof(uint8)));
    }

    void IslandBuilder::GetBodiesInIsland(uint32 islandIndex, BodyID*& outBodiesBegin, BodyID*& outBodiesEnd) const
    {
        NES_ASSERT(islandIndex < m_numIslands);
        const uint32 sortedIndex = m_islandsSorted != nullptr ? m_islandsSorted[islandIndex] : islandIndex;
        outBodiesBegin = sortedIndex > 0? m_bodyIslands + m_bodyIslandEnds[sortedIndex - 1] : m_bodyIslands;
        outBodiesEnd = m_bodyIslands + m_bodyIslandEnds[sortedIndex];
    }

    bool IslandBuilder::GetConstraintsInIsland(uint32 islandIndex, uint32*& outConstraintsBegin, uint32*& outConstraintsEnd) const
    {
        NES_ASSERT(islandIndex < m_numIslands);
        if (m_numConstraints == 0)
        {
            outConstraintsBegin = nullptr;
            outConstraintsEnd = nullptr;
            return false;
        }
        
        const uint32 sortedIndex = m_islandsSorted[islandIndex];
        outConstraintsBegin = sortedIndex > 0? m_constraintIslands + m_constraintIslandEnds[sortedIndex - 1] : m_constraintIslands;
        outConstraintsEnd = m_constraintIslands + m_constraintIslandEnds[sortedIndex];
        return outConstraintsBegin != outConstraintsEnd;
    }

    bool IslandBuilder::GetContactsInIsland(uint32 islandIndex, uint32*& outContactsBegin, uint32*& outContactsEnd) const
    {
        NES_ASSERT(islandIndex < m_numIslands);
        if (m_numContacts == 0)
        {
            outContactsBegin = nullptr;
            outContactsEnd = nullptr;
            return false;
        }
        
        const uint32 sortedIndex = m_islandsSorted[islandIndex];
        outContactsBegin = sortedIndex > 0? m_contactIslands + m_contactIslandEnds[sortedIndex - 1] : m_contactIslands;
        outContactsEnd = m_contactIslands + m_contactIslandEnds[sortedIndex];
        return outContactsBegin != outContactsEnd;
    }

    void IslandBuilder::SetNumPositionSteps(const uint32 islandIndex, const uint numPositionSteps)
    {
        NES_ASSERT(islandIndex < m_numIslands);
        NES_ASSERT(numPositionSteps < 256);
        m_numPositionSteps[islandIndex] = static_cast<uint8>(numPositionSteps);
    }

    uint IslandBuilder::GetNumPositionSteps(const uint32 islandIndex) const
    {
        NES_ASSERT(islandIndex < m_numIslands);
        return m_numPositionSteps[islandIndex];
    }

    void IslandBuilder::ResetIslands(StackAllocator* pAllocator)
    {
        pAllocator->Free(m_numPositionSteps, m_numIslands * sizeof(uint8));

        // Sorted Islands
        if (m_islandsSorted != nullptr)
        {
            pAllocator->Free(m_islandsSorted, m_numIslands * sizeof(uint32));
            m_islandsSorted = nullptr;
        }

        // Contacts
        if (m_contactIslands != nullptr)
        {
            pAllocator->Free(m_contactIslandEnds, (m_numIslands + 1) * sizeof(uint32));
            m_contactIslandEnds = nullptr;
            pAllocator->Free(m_contactIslands, m_numContacts * sizeof(uint32));
            m_contactIslands = nullptr;
        }

        // Constraints
        if (m_constraintIslands != nullptr)
        {
            pAllocator->Free(m_constraintIslandEnds, (m_numIslands + 1) * sizeof(uint32));
            m_constraintIslandEnds = nullptr;
            pAllocator->Free(m_constraintIslands, m_numConstraints * sizeof(uint32));
            m_constraintIslands = nullptr;
        }
        
        // Islands
        pAllocator->Free(m_bodyIslandEnds, (m_numActiveBodies + 1) * sizeof(uint32));
        m_bodyIslandEnds = nullptr;
        pAllocator->Free(m_bodyIslands, m_numActiveBodies * sizeof(uint32));
        m_bodyIslands = nullptr;

        // Constraint Links
        pAllocator->Free(m_constraintLinks, m_numConstraints * sizeof(uint32));
        m_constraintLinks = nullptr;

        // Contact Links
        pAllocator->Free(m_contactLinks, m_maxContacts * sizeof(uint32));
        m_contactLinks = nullptr;

        // Reset Counters.
        m_numActiveBodies = 0;
        m_numConstraints = 0;
        m_maxContacts = 0;
        m_numContacts = 0;
        m_numIslands = 0;
    }

    uint32 IslandBuilder::GetLowestBodyIndex(const uint32 activeBodyIndex) const
    {
        // Find the body that is linked to itself, that is the first body in the island.
        uint32 index = activeBodyIndex;
        for (;;)
        {
            const uint32 linkTo = m_bodyLinks[index].m_linkedTo.load(std::memory_order_relaxed);
            if (linkTo == index)
                break;
            
            index = linkTo;
        }

        return index;
    }

    void IslandBuilder::BuildBodyIslands(const BodyID* pActiveBodies, uint32 numActiveBodies, StackAllocator* pAllocator)
    {
        // Store the number of active bodies.
        m_numActiveBodies = numActiveBodies;

        // Create the output arrays for the body ID's; don't call constructors.
        NES_ASSERT(m_bodyIslands == nullptr);
        m_bodyIslands = static_cast<BodyID*>(pAllocator->Allocate(numActiveBodies * sizeof(BodyID)));

        // Create the output array for the start index of each island. At this point, we don't know how many islands
        // there will be, but we know it cannot be more than numActiveBodies.
        // Note: We allocate 1 extra entry because we always increment the count of the next island.
        uint32* bodyIslandStarts = static_cast<uint32*>(pAllocator->Allocate((numActiveBodies + 1) * sizeof(uint32)));

        // The first island always starts at 0.
        bodyIslandStarts[0] = 0;

        // Calculate the island index for all bodies.
        NES_ASSERT(m_numIslands == 0);
        for (uint32 i = 0; i < numActiveBodies; ++i)
        {
            BodyLink& link = m_bodyLinks[i];
            const uint32 linkTo = link.m_linkedTo.load(std::memory_order_relaxed);
            if (linkTo != i)
            {
                // Links to another body, take island index from other body (This must have been filled in already since we're looping from
                // low to high.
                NES_ASSERT(linkTo < i);
                const uint32 islandIndex = m_bodyLinks[linkTo].m_islandIndex;
                link.m_islandIndex = islandIndex;

                // Increment the start of the next island.
                bodyIslandStarts[islandIndex + 1]++; 
            }
            else
            {
                // Does not link to other body, this is the start of a new island.
                link.m_islandIndex = m_numIslands;
                ++m_numIslands;

                // Set the start of the next island to 1.
                bodyIslandStarts[m_numIslands] = 1;
            }
        }

        // Make the start array absolute (so far we only counted).
        for (uint32 island = 1; island < m_numIslands; ++island)
        {
            bodyIslandStarts[island] += bodyIslandStarts[island - 1];
        }

        // Convert to a linear list grouped by island
        for (uint32 i = 0; i < numActiveBodies; ++i)
        {
            BodyLink& link = m_bodyLinks[i];

            // Copy the body to the correct location in the array and increment it.
            uint32& start = bodyIslandStarts[link.m_islandIndex];
            m_bodyIslands[start] = pActiveBodies[i];
            ++start;

            // Reset linked to field for the next update
            link.m_linkedTo.store(i, std::memory_order_relaxed);
        }

        // We should now have a full array
        NES_ASSERT(m_numIslands == 0 || bodyIslandStarts[m_numIslands - 1] == numActiveBodies);

        // We've incremented all body indices so that they now point at the end instead of the starts.
        NES_ASSERT(m_bodyIslandEnds == nullptr);
        m_bodyIslandEnds = bodyIslandStarts;
    }

    void IslandBuilder::BuildConstraintIslands(const uint32* pConstraintToBody, uint32 numConstraints, uint32*& outConstraints, uint32*& outConstraintEnds, StackAllocator* pAllocator) const
    {
        // Return if there is nothing to do.
        if (numConstraints == 0)
            return;

        // Creat the output arrays for the constraints.
        // Note: for the end indices, we allocate 1 extra entry so that we don't have to do an 'if' in the inner loop.
        uint32* constraints = static_cast<uint32*>(pAllocator->Allocate(numConstraints * sizeof(uint32)));
        uint32* constraintEnds = static_cast<uint32*>(pAllocator->Allocate((m_numIslands + 1) * sizeof(uint32)));

        // Reset sizes.
        for (uint32 island = 0; island < m_numIslands; ++island)
        {
            constraintEnds[island] = 0;
        }

        // Loop over the array and increment the start relative position for the next island.
        for (uint32 constraint = 0; constraint < numConstraints; ++constraint)
        {
            const uint32 bodyIndex = pConstraintToBody[constraint];
            const uint32 nextIslandIndex = m_bodyLinks[bodyIndex].m_islandIndex + 1;
            NES_ASSERT(nextIslandIndex <= m_numIslands);
            constraintEnds[nextIslandIndex]++;
        }

        // Make the start positions absolute.
        for (uint32 island = 1; island < m_numIslands; ++island)
        {
            constraintEnds[island] += constraintEnds[island - 1];
        }

        // Loop over the array and collect constraints
        for (uint32 constraint = 0; constraint < numConstraints; ++constraint)
        {
            const uint32 bodyIndex = pConstraintToBody[constraint];
            const uint32 islandIndex = m_bodyLinks[bodyIndex].m_islandIndex;
            constraints[constraintEnds[islandIndex]++] = constraint;
        }

        NES_ASSERT(outConstraints == nullptr);
        outConstraints = constraints;
        NES_ASSERT(outConstraintEnds == nullptr);
        outConstraintEnds = constraintEnds;
    }

    void IslandBuilder::SortIslands(StackAllocator* pAllocator)
    {
        if (m_numContacts > 0 || m_numConstraints > 0)
        {
            // Allocate mapping table
            NES_ASSERT(m_islandsSorted == nullptr);
            m_islandsSorted = static_cast<uint32*>(pAllocator->Allocate((m_numIslands) * sizeof(uint32)));

            // Initialize indices
            for (uint32 island = 0; island < m_numIslands; ++island)
            {
                m_islandsSorted[island] = island;
            }

            // Determine the sum of contact constraints / constraints per island
            uint32* numConstraints = static_cast<uint32*>(pAllocator->Allocate(m_numIslands * sizeof(uint32)));
            if (m_numContacts > 0 && m_numConstraints > 0)
            {
                numConstraints[0] = m_constraintIslandEnds[0] + m_contactIslandEnds[0];
                for (uint32 island = 1; island < m_numIslands; ++island)
                {
                    numConstraints[island] = m_constraintIslandEnds[island] - m_constraintIslandEnds[island - 1]
                                           + m_contactIslandEnds[island] - m_contactIslandEnds[island - 1];
                }
            }
            else if (m_numContacts > 0)
            {
                numConstraints[0] = m_contactIslandEnds[0];
                for (uint32 island = 1; island < m_numIslands; ++island)
                {
                    numConstraints[island] = m_contactIslandEnds[island] - m_contactIslandEnds[island - 1];
                }
            }
            else
            {
                numConstraints[0] = m_constraintIslandEnds[0];
                for (uint32 island = 1; island < m_numIslands; ++island)
                {
                    numConstraints[island] = m_constraintIslandEnds[island] - m_constraintIslandEnds[island - 1];
                }
            }

            // Sort so the biggest islands go first, this means that the jobs that take longest will be running
            // first which improves the chance that all jobs finish at the same time.
            QuickSort(m_islandsSorted, m_islandsSorted + m_numIslands, [numConstraints](const uint32 left, const uint32 right)
            {
                return numConstraints[left] > numConstraints[right]; 
            });

            pAllocator->Free(numConstraints, m_numIslands * sizeof(uint32));
        }
    }
}
