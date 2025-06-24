// BodyFilter.h
#pragma once

#include "BodyID.h"

namespace nes
{
    class Body;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Class with functions to filter out bodies. Tests return true if we should collide the given body. 
    //----------------------------------------------------------------------------------------------------
    class BodyFilter
    {
    public:
        BodyFilter() = default;
        BodyFilter(const BodyFilter&) = delete;
        BodyFilter& operator=(const BodyFilter&) = delete;
        virtual ~BodyFilter() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filter function. Returns true if we should collide with the body with given BodyID.
        //----------------------------------------------------------------------------------------------------
        virtual bool ShouldCollide([[maybe_unused]] const BodyID& bodyID) const { return true; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filter function. Returns true if we should collide with the given body.
        /// This is called after the body has been locked and makes it possible to filter based on Body members.
        //----------------------------------------------------------------------------------------------------
        virtual bool ShouldCollideLocked([[maybe_unused]] const Body& body) const { return true; }
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A simple body filter implementation that ignores a single, specified body. 
    //----------------------------------------------------------------------------------------------------
    class IgnoreSingleBodyFilter : public BodyFilter
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Constructor. Pass in the the body you want to ignore. 
        //----------------------------------------------------------------------------------------------------
        explicit        IgnoreSingleBodyFilter(const BodyID& bodyID) : m_bodyID(bodyID) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filter function. Returns true if we should collide with the body.
        //----------------------------------------------------------------------------------------------------
        virtual bool    ShouldCollide(const BodyID& bodyID) const override { return bodyID != m_bodyID; }

    private:
        BodyID          m_bodyID; 
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A simple body filter implementation that ignores multiple specified bodies.
    //----------------------------------------------------------------------------------------------------
    class IgnoreMultipleBodiesFilter : public BodyFilter
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Remove all bodies from the filter. 
        //----------------------------------------------------------------------------------------------------
        void                Clear() { m_bodyIDs.clear(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Reserve space for 'size' body ID's.
        //----------------------------------------------------------------------------------------------------
        void                Reserve(const uint size) { m_bodyIDs.reserve(size); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Add a body to be ignored. 
        //----------------------------------------------------------------------------------------------------
        void                IgnoreBody(const BodyID& bodyID) { m_bodyIDs.push_back(bodyID); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filter function. Returns true if we should collide with the body.
        //----------------------------------------------------------------------------------------------------
        virtual bool        ShouldCollide(const BodyID& bodyID) const override { return std::find(m_bodyIDs.begin(), m_bodyIDs.end(), bodyID) == m_bodyIDs.end(); }
    
    private:
        std::vector<BodyID> m_bodyIDs;
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Ignores a single body and chains the filter to another filter. 
    //----------------------------------------------------------------------------------------------------
    class IgnoreSingleBodyFilterChained : public BodyFilter
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Constructor. Pass in the body ID that you want to ignore and the filter that you want to
        ///     check as well.
        //----------------------------------------------------------------------------------------------------
        explicit            IgnoreSingleBodyFilterChained(const BodyID& bodyID, const BodyFilter& filter) : m_bodyID(bodyID), m_filter(filter) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filter function. Returns true if we should collide with the body.
        //----------------------------------------------------------------------------------------------------
        virtual bool        ShouldCollide(const BodyID& bodyID) const override { return bodyID != m_bodyID && m_filter.ShouldCollide(bodyID); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Filter function.  Returns true if we should collide with the given body.
        /// This is called after the body has been locked and makes it possible to filter based on Body members.
        //----------------------------------------------------------------------------------------------------
        virtual bool        ShouldCollideLocked(const Body& body) const override { return m_filter.ShouldCollideLocked(body); }

    private:
        BodyID              m_bodyID;
        const BodyFilter&   m_filter;
    };
}