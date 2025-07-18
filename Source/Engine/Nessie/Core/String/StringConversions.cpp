// StringConversions.cpp
#include "StringConversions.h"

namespace nes
{
    void ConvertCharToWchar(const char* in, wchar_t* out, const size_t outLength)
    {
        if (outLength == 0)
            return;

        for (size_t i = 0; i < outLength - 1 && *in; i++)
        {
            *out++ = *in++;
        }

        *out = 0;
    }

    void ConvertWcharToChar(const wchar_t* in, char* out, const size_t outLength)
    {
        if (outLength == 0)
            return;

        for (size_t i = 0; i < outLength - 1 && *in; i++)
        {
            *out++ = static_cast<char>(*in++);
        }

        *out = 0;
    }
}
