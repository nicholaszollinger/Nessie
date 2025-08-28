// ApplicationDesc.cpp
#include "ApplicationDesc.h"

namespace nes
{
    ApplicationDesc& ApplicationDesc::SetApplicationName(const std::string& appName)
    {
        m_appName = appName;
        return *this;
    }

    ApplicationDesc& ApplicationDesc::SetApplicationVersion(const Version& appVersion)
    {
        m_appVersion = appVersion;
        return *this;
    }

    ApplicationDesc& ApplicationDesc::SetMinTimeStep(const float minTimeStepMs)
    {
        m_minTimeStepMs = minTimeStepMs;
        return *this;
    }

    ApplicationDesc& ApplicationDesc::SetIsHeadless(const bool isHeadless, const uint32 numFrames)
    {
        m_isHeadless = isHeadless;
        m_headlessFrameCount = math::Max(1U, numFrames);
        return *this;
    }
}
