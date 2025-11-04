// YamlSerializers.h
#pragma once
#include "YamlReader.h"
#include "YamlWriter.h"

namespace nes
{
    struct Camera;
    struct CameraSerializer
    {
        static void Deserialize(const YamlNode& in, Camera& camera);
        static void Serialize(YamlWriter& out, const Camera& camera);
    };
}