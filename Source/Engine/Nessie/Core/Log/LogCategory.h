// LogCategory.h
#pragma once
#include "Core/Config.h"
#include "Core/String/StringID.h"

namespace nes
{
    //----------------------------------------------------------------------------------------------------
    /// @brief : Determines where a log will be written/displayed to when called.
    //----------------------------------------------------------------------------------------------------
    enum class ELogOutputLevel : uint8
    {
        None        = 0,    /// The log will not be visible or written to anywhere.
        LogTarget   = 1,    /// The log will be displayed in the current Log Target.
        File        = 2,    /// The log will be written to a file.
        All         = 3,    /// The log will be displayed and written to a file.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : A log category contains a name and an output level. This can be used to group logs and 
    ///     control their visibility.
    //----------------------------------------------------------------------------------------------------
    class LogCategory
    {
    public:
        LogCategory() = default;
        LogCategory(const StringID name, const ELogOutputLevel outputLevel) : m_name(name), m_output(outputLevel) {}
        LogCategory(const LogCategory&) = default;
        LogCategory(LogCategory&&) noexcept = default;
        LogCategory& operator=(const LogCategory&) = default;
        LogCategory& operator=(LogCategory&&) noexcept = default;
        ~LogCategory() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the name of this category.
        //----------------------------------------------------------------------------------------------------
        const std::string&  GetName() const { return m_name.StringRef(); }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the output level of this category.
        //----------------------------------------------------------------------------------------------------
        ELogOutputLevel     GetOutputLevel() const { return m_output; }

    private:
        StringID            m_name;
        ELogOutputLevel     m_output = ELogOutputLevel::None;
    };
} 

