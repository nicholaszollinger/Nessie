// StringConversions.h
#pragma once

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Convert from char* to wchar_t* string types.  
    ///	@param in : String to read from.
    ///	@param out : Output string.
    ///	@param outLength : How long the output string will be.
    //----------------------------------------------------------------------------------------------------
    void ConvertCharToWchar(const char* in, wchar_t* out, const size_t outLength);

    //----------------------------------------------------------------------------------------------------
    /// @brief : Convert from wchar_t* to char* string types.  
    ///	@param in : String to read from.
    ///	@param out : Output string.
    ///	@param outLength : How long the output string will be.
    //----------------------------------------------------------------------------------------------------
    void ConvertWcharToChar(const wchar_t* in, char* out, const size_t outLength);
}