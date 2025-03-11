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
///		@brief : Two World Domains are compatible only if they are equal, or if either are the Abstract
///         Domain.
//----------------------------------------------------------------------------------------------------
static constexpr bool DomainsAreCompatible(const EntityDomain a, const EntityDomain b)
{
    if (a == b || a == EntityDomain::Abstract || b == EntityDomain::Abstract)
        return true;

    return false;
}