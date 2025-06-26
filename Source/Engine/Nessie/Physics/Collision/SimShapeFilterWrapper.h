// SimShapeFilterWrapper.h
#pragma once

#include "Physics/Collision/SimShapeFilter.h"
#include "Physics/Body/Body.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Helper class to forward ShapeFilter calls to a SimShapeFilter.
    /// @note : Internal class! Do not use!
    //----------------------------------------------------------------------------------------------------
    class Internal_SimShapeFilterWrapper : private ShapeFilter
    {
    public:
        Internal_SimShapeFilterWrapper(const SimShapeFilter* pFilter, const Body* pBody1)
            : m_pFilter(pFilter)
            , m_pBody1(pBody1)
        {
            // Fall back to an empty filter if no simulation shape filter is set. This reduces the virtual call to 'return true'.
            m_pFinalFilter = pFilter == nullptr ? this : &m_default;
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Forwards to the simulation shape filter. 
        //----------------------------------------------------------------------------------------------------
        virtual bool            ShouldCollide(const Shape* pShape1, const SubShapeID& subShapeIDOfShape1, const Shape* pShape2, const SubShapeID& subShapeIDOfShape2) const override
        {
            return m_pFilter->ShouldCollide(*m_pBody1, pShape1, subShapeIDOfShape1, *m_pBody2, pShape2, subShapeIDOfShape2);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Forwards to the simulation shape filter. 
        //----------------------------------------------------------------------------------------------------
        virtual bool            ShouldCollide(const Shape* pShape2, const SubShapeID& subShapeIDOfShape2) const override
        {
            return m_pFilter->ShouldCollide(*m_pBody1, m_pBody1->GetShape(), SubShapeID(), *m_pBody2, pShape2, subShapeIDOfShape2);
        }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the body we're colliding against.
        //----------------------------------------------------------------------------------------------------
        void                    SetBody2(const Body* pBody2) { m_pBody2 = pBody2; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns the actual filter to use for collision detection. 
        //----------------------------------------------------------------------------------------------------
        const ShapeFilter&      GetFilter() const { return *m_pFinalFilter; }

    private:
        const ShapeFilter*      m_pFinalFilter;
        const SimShapeFilter*   m_pFilter;
        const Body*             m_pBody1;
        const Body*             m_pBody2 = nullptr;
        const ShapeFilter       m_default;
    };
}