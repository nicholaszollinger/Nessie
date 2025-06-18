// CustomFlagFormatter.h
#pragma once
#include "FlagFormatter.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for custom formatter that replaces a char symbol with a string in a log message.
    //----------------------------------------------------------------------------------------------------
    class CustomFlagFormatter : public internal::FlagFormatter
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Create a unique clone of this CustomFlagFormatter. 
        //----------------------------------------------------------------------------------------------------
        virtual std::unique_ptr<CustomFlagFormatter> Clone() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set the padding info for this formatter. 
        //----------------------------------------------------------------------------------------------------
        void SetPaddingInfo(const internal::PaddingInfo paddingInfo) { m_paddingInfo = paddingInfo; }
    };
}