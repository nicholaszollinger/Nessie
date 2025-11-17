// YamlGraphicsSerializers.h
#pragma once
#include "Nessie/FileIO/YAML/YamlSerializer.h"
#include "Nessie/Graphics/GraphicsCommon.h"
#include "Nessie/Graphics/Camera.h"

//============================================================================================================================================================================================
#pragma region [ Common ]
//============================================================================================================================================================================================

namespace nes
{
    struct CameraSerializer
    {
        static void Deserialize(const YamlNode& in, Camera& camera);
        static void Serialize(YamlOutStream& out, const Camera& camera);
    };
}

#pragma endregion
//============================================================================================================================================================================================