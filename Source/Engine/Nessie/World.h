// World.h
// - Single header for World functionality, as well as the most common components.  
#pragma once

#include "World/EntityRegistry.h"
#include "World/ComponentRegistry.h"
#include "World/ComponentSystem.h"
#include "World/WorldBase.h"

// Common Components:
#include "World/Components/IDComponent.h"
#include "World/Components/LifetimeComponents.h"
#include "World/Components/NodeComponent.h"


// Implementation files:
#include "World/EntityRegistry.inl"
#include "World/ComponentRegistry.inl"
#include "World/WorldBase.inl"