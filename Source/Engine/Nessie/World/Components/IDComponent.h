// EntityPropsComponent.h
#pragma once
#include <string>
#include "Nessie/World/Component.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : An Entity ID is a unique identifier that can be saved and loaded.
    //----------------------------------------------------------------------------------------------------
    using EntityID = uint64;
    static constexpr EntityID kInvalidEntityID = 0;
    
    //----------------------------------------------------------------------------------------------------
    /// @brief : This is a component that is added to each entity by default. Contains a globally unique identifier
    /// and an optional name.
    //----------------------------------------------------------------------------------------------------
    class IDComponent
    {
    public:
        IDComponent(const std::string& name = {});
        IDComponent(const uint64 id, const std::string& name = {}) : m_name(name), m_id(id) {}
        IDComponent(const IDComponent& other) = default;
        IDComponent& operator=(const IDComponent& other) = default;
        IDComponent(IDComponent&& other) noexcept;
        IDComponent& operator=(IDComponent&& other) noexcept;
        ~IDComponent() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the serializable ID for the Entity.
        //----------------------------------------------------------------------------------------------------
        EntityID            GetID() const                                               { return m_id; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the name of the Entity. Does not have to be unique.
        //----------------------------------------------------------------------------------------------------
        void                SetName(const std::string& name)                            { m_name = name; }
        const std::string&  GetName() const                                             { return m_name;}
    
    private:
        std::string         m_name{};
        EntityID            m_id = 0;
    };

    static_assert(ComponentType<IDComponent>);
}