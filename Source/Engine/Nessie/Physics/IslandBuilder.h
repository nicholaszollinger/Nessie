// IslandBuilder.h
#pragma once
#include "Nessie/Core/Thread/Atomics.h"
#include "Nessie/Physics/Body/BodyID.h"

namespace nes
{
    class StackAllocator;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Keeps track of connected bodies and builds islands for multithreaded velocity/position
    ///     update.
    //----------------------------------------------------------------------------------------------------
    class IslandBuilder
    {
    public:
        IslandBuilder() = default;
        IslandBuilder(const IslandBuilder&) = delete;
        IslandBuilder& operator=(const IslandBuilder&) = delete;
        ~IslandBuilder();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Initialize the island builder with the maximum number of bodies that could be active.
        //----------------------------------------------------------------------------------------------------
        void                    Init(const uint32 maxActiveBodies);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Prepare for the simulation step by allocating space for the contact constraints.
        //----------------------------------------------------------------------------------------------------
        void                    PrepareContactConstraints(const uint32 maxContacts, StackAllocator* pAllocator);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Prepare for the simulation step by allocating space for the non-contact constraints. 
        //----------------------------------------------------------------------------------------------------
        void                    PrepareNonContactConstraints(const uint32 numConstraints, StackAllocator* pAllocator);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Link two bodies by their index int he BodyManager::m_activeBodies list to form islands.
        //----------------------------------------------------------------------------------------------------
        void                    LinkBodies(const uint32 first, const uint32 second);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Link a constraint to a body pair by their index in the BodyManager::m_activeBodies array. 
        //----------------------------------------------------------------------------------------------------
        void                    LinkConstraint(const uint32 constraintIndex, uint32 first, uint32 second);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Link a contact to a body pair by their index in the BodyManager::m_activeBodies array.
        //----------------------------------------------------------------------------------------------------
        void                    LinkContact(const uint32 contactIndex, uint32 first, uint32 second);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Finalize the islands after all bodies have been linked.
        //----------------------------------------------------------------------------------------------------
        void                    Finalize(const BodyID* pActiveBodies, uint32 numActiveBodies, uint32 numContacts, StackAllocator* pAllocator);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of islands formed after calling Finalize(). 
        //----------------------------------------------------------------------------------------------------
        uint32                  GetNumIslands() const       { return m_numIslands; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a start and end iterator to the bodies in a particular island.
        //----------------------------------------------------------------------------------------------------
        void                    GetBodiesInIsland(uint32 islandIndex, BodyID*& outBodiesBegin, BodyID*& outBodiesEnd) const;
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a start and end iterator to the constraints in a particular island. Returns false if there
        ///     are no constraints.
        //----------------------------------------------------------------------------------------------------
        bool                    GetConstraintsInIsland(uint32 islandIndex, uint32*& outConstraintsBegin, uint32*& outConstraintsEnd) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get a start and end iterator to the contacts in a particular island. Returns false if there
        ///     are no contacts.
        //----------------------------------------------------------------------------------------------------
        bool                    GetContactsInIsland(uint32 islandIndex, uint32*& outContactsBegin, uint32*& outContactsEnd) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the number of position solver steps for a particular island. Must be less than 256. 
        //----------------------------------------------------------------------------------------------------
        void                    SetNumPositionSteps(uint32 islandIndex, uint numPositionSteps);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the number of position solver steps for a particular island.
        //----------------------------------------------------------------------------------------------------
        uint                    GetNumPositionSteps(uint32 islandIndex) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : After you're done using the built island data, call this to free the associated data. 
        //----------------------------------------------------------------------------------------------------
        void                    ResetIslands(StackAllocator* pAllocator);

    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the index of the lowest body in the group. 
        //----------------------------------------------------------------------------------------------------
        uint32                  GetLowestBodyIndex(uint32 activeBodyIndex) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function to build the body islands.
        //----------------------------------------------------------------------------------------------------
        void                    BuildBodyIslands(const BodyID* pActiveBodies, uint32 numActiveBodies, StackAllocator* pAllocator);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Helper function to build the constraint islands.
        //----------------------------------------------------------------------------------------------------
        void                    BuildConstraintIslands(const uint32* pConstraintToBody, uint32 numConstraints, uint32*& outConstraints, uint32*& outConstraintEnds, StackAllocator* pAllocator) const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Sorts the islands so that the islands with the most constraints are first.  
        //----------------------------------------------------------------------------------------------------
        void                    SortIslands(StackAllocator* pAllocator);
        
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Intermediate data structure that keeps track what the lowest index of the body that a
        ///     body is connected to.
        //----------------------------------------------------------------------------------------------------
        struct BodyLink
        {
            std::atomic<uint32> m_linkedTo;                         /// Index in m_bodyLinks pointing to another body in this island with a lower index than this body.
            uint32              m_islandIndex;                      /// The island index of this body (set during Finalize()).
        };

        // Intermediate data
        BodyLink*               m_bodyLinks = nullptr;              /// Maps bodies to the first body in the island.
        uint32*                 m_constraintLinks = nullptr;        /// Maps constraint index to the body index (which maps to island index).
        uint32*                 m_contactLinks = nullptr;           /// Maps contact constraint index to body index (which maps to island index).

        // Final Data
        BodyID*                 m_bodyIslands = nullptr;            /// Bodies ordered by island.
        uint32*                 m_bodyIslandEnds = nullptr;         /// End index of each body island.
        uint32*                 m_constraintIslands = nullptr;      /// Constraints ordered by island.
        uint32*                 m_constraintIslandEnds = nullptr;   /// End index of each constraint island.
        uint32*                 m_contactIslands = nullptr;         /// Contacts ordered by island.
        uint32*                 m_contactIslandEnds = nullptr;      /// End index of each contact island.
        uint32*                 m_islandsSorted = nullptr;          /// A list of island indices in order of most constraints first.
        uint8*                  m_numPositionSteps = nullptr;       /// Number of position steps for each island.

        // Counters
        uint32                  m_maxActiveBodies;                  /// Maximum size of the active bodies array (see BodyManager::m_activeBodies).
        uint32                  m_numActiveBodies = 0;              /// Number of active bodies passed to.
        uint32                  m_numConstraints = 0;               /// Size of the contact constraints array (see ConstraintManager::m_constraints).
        uint32                  m_maxContacts = 0;                  /// Maximum number of contacts supported.
        uint32                  m_numContacts = 0;                  /// Size of the contacts array (see ContactConstraintsManager::m_numConstraints).
        uint32                  m_numIslands = 0;                   /// Final number of islands.
    };
}