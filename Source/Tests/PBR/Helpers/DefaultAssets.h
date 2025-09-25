// DefaultAssets.h
#pragma once
#include "Nessie/Asset/AssetBase.h"

enum class EDefaultTextureType : uint8
{
    Error,      // Magenta and Black Checkerboard
    Black,      // All pixels Black
    White,      // All pixels White
    FlatNormal, // All pixels = (127, 127, 255)
};

enum class EDefaultMeshType : uint8
{
    Cube,
    Plane,
    Sphere,
};

namespace helpers
{
    static constexpr std::array<nes::AssetID, 4> kDefaultTextureIDs =
    {
        10, // Error
        11, // Black
        12, // White
        13, // FlatNormal
    };

    static constexpr std::array<nes::AssetID, 3> kDefaultMeshIDs =
    {
        1, // Cube
        2, // Plane
        3, // Sphere
    };
    
    constexpr nes::AssetID GetDefaultTextureID(const EDefaultTextureType type)
    {
        return kDefaultTextureIDs[static_cast<uint8>(type)];
    }
    
    constexpr nes::AssetID GetDefaultMeshID(const EDefaultMeshType type)
    {
        return kDefaultMeshIDs[static_cast<uint8>(type)];
    }

    constexpr nes::AssetID GetDefaultMaterialID()
    {
        return 5000;
    }
}
