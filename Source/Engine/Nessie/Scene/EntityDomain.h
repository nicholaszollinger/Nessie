// WorldDomain.h
#pragma once
#include <cstdint>
#include "Debug/Assert.h"

//----------------------------------------------------------------------------------------------------
///		@brief : An Entity Domain describes how an Entity exists in a Scene. Does this entity exist in
///         3D space? 2D Space? Is it Abstract? etc.
//----------------------------------------------------------------------------------------------------
enum class EntityDomain : uint8_t
{
    Abstract = 0,   // Entities that exist, but not tangible by the User. Ex: GameManager 
    Physical2D,     // Entities that exist in 2D space.
    Physical3D,     // Entities that exist in 3D space.
    Screen,         // Entities that exist in Screen space, like UI.
};

static constexpr const char* GetWorldDomainName(const EntityDomain value)
{
    switch (value)
    {
        case EntityDomain::Abstract:    return "Abstract";
        case EntityDomain::Physical2D:  return "Physical2D";
        case EntityDomain::Physical3D:  return "Physical3D";
        case EntityDomain::Screen:      return "Screen";
        
        default:
        {
            NES_ASSERT(false);
            return "Invalid Domain!";
        }
    }
}

//----------------------------------------------------------------------------------------------------
///		@brief : An Entity's Domain is compatible with a Component's Domain if they are equal or
///         if the Component Domain is Abstract. Components in the Abstract Domain can be added to
///         Entities in any Domain. 
//----------------------------------------------------------------------------------------------------
static constexpr bool ComponentDomainIsCompatibleForEntity(const EntityDomain entityDomain, const EntityDomain componentDomain)
{
    if (entityDomain == componentDomain || componentDomain == EntityDomain::Abstract)
        return true;

    return false;
}