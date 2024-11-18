// FormatString.cpp
#include "FormatString.h"

namespace nes
{
    //-----------------------------------------------------------------------------------------------------------------------------
    ///		@brief : Our recursive end to the FormatStringImpl. Upon getting to this overload, our formatting is finished, and we can just add the last bit
    ///             of the format string to the outStr.
    //-----------------------------------------------------------------------------------------------------------------------------
    void FormatStringImpl(const char* pFormat, std::string& outStr)
    {
        outStr += pFormat;
    }
}