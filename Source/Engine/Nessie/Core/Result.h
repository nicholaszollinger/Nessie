// Result.h
#pragma once
#include "Nessie/Core/Concepts.h"
#include "Nessie/Debug/Assert.h"

namespace nes
{
    template <typename Type>
    class Result
    {
        static_assert(std::is_default_constructible_v<Type>
            && std::is_copy_constructible_v<Type>
            && std::is_move_constructible_v<Type>
            , "Result Type must be default, copy, & move constructible!");
        
        enum class EState : uint8_t
        {
            Invalid,    /// The result is uninitialized
            Valid,      /// The result is valid.
            Error,      /// The result is invalid, and an error string has been set.
        };

    public:
        Result(){}
        Result(const Result& other);
        Result(Result&& other) noexcept;
        ~Result()                           { Clear(); }

        inline Result& operator=(const Result& other);
        inline Result& operator=(Result&& other) noexcept;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Destructs either the result or error message. IsEmpty() will return true after this.  
        //----------------------------------------------------------------------------------------------------
        inline void         Clear();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if there is a result value set.
        //----------------------------------------------------------------------------------------------------
        inline bool         IsValid() const             { return m_state == EState::Valid; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns true if neither an error or result value was set. 
        //----------------------------------------------------------------------------------------------------
        inline bool         IsEmpty() const             { return m_state == EState::Invalid; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Checks if there is was an error message set. 
        //----------------------------------------------------------------------------------------------------
        inline bool         HasError() const            { return m_state == EState::Error; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the value result value. YUou must check IsValid() first. 
        //----------------------------------------------------------------------------------------------------
        inline const Type&  Get() const                 { NES_ASSERT(IsValid()); return m_result; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a result value. IsValid() will return true. 
        //----------------------------------------------------------------------------------------------------
        inline void         Set(const Type& value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set a result value. IsValid() will return true. 
        //----------------------------------------------------------------------------------------------------
        inline void         Set(Type&& value);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the current Error value. You must check HasError() first. 
        //----------------------------------------------------------------------------------------------------
        const std::string&  GetError() const            { NES_ASSERT(HasError()); return m_error; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an error value. This will destruct current result if there was one.
        //----------------------------------------------------------------------------------------------------
        inline void         SetError(const char* errorMsg);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an error value. This will destruct current result if there was one.
        //----------------------------------------------------------------------------------------------------
        inline void         SetError(const std::string& errorMsg);
        
        //----------------------------------------------------------------------------------------------------
        /// @brief : Set an error value. This will destruct current result if there was one.
        //----------------------------------------------------------------------------------------------------
        inline void         SetError(std::string&& errorMsg);

    private:
        union
        {
            Type        m_result{};      /// The actual resulting object.
            std::string m_error;  /// The error description on failure.
        };

        EState          m_state = EState::Invalid;
    };

    template <typename Type>
    Result<Type>::Result(const Result& other)
        : m_state(other.m_state)
    {
        switch (m_state)
        {
            case EState::Valid:
            {
                new (&m_result) Type (other.m_result);
                break;
            }

            case EState::Error:
            {
                new (&m_error) std::string (other.m_error);
                break;
            }
            
            default: break;
        }
    }

    template <typename Type>
    Result<Type>::Result(Result&& other) noexcept
        : m_state(other.m_state)
    {
        m_state = other.m_state;
        
        switch (m_state)
        {
            case EState::Valid:
            {
                new (&m_result) Type (std::move(other.m_result));
                break;
            }

            case EState::Error:
            {
                new (&m_error) std::string (std::move(other.m_error));
                break;
            }
            
            default: break;
        }

        // ? 
        // We don't reset the state of the other result. The destructors still need to be called after a move
        // operation.
    }

    template <typename Type>
    Result<Type>& Result<Type>::operator=(const Result& other)
    {
        if (this != &other)
        {
            Clear();
            m_state = other.m_state;
            
            switch (m_state)
            {
                case EState::Valid:
                {
                    new (&m_result) Type (other.m_result);
                    break;
                }

                case EState::Error:
                {
                    new (&m_error) std::string (other.m_error);
                    break;
                }
            
                default: break;
            }
        }

        return *this;
    }

    template <typename Type>
    Result<Type>& Result<Type>::operator=(Result&& other) noexcept
    {
        if (this != &other)
        {
            Clear();
            m_state = other.m_state;
        
            switch (m_state)
            {
                case EState::Valid:
                {
                    new (&m_result) Type (std::move(other.m_result));
                    break;
                }

                case EState::Error:
                {
                    new (&m_error) std::string (std::move(other.m_error));
                    break;
                }
            
                default: break;
            }

            // ? 
            // We don't reset the state of the other result. The destructors still need to be called after a move
            // operation.
        }
        
        return *this;
    }

    template <typename Type>
    void Result<Type>::Clear()
    {
        switch (m_state)
        {
            case EState::Valid:
            {
                m_result.~Type();
                break;
            }

            case EState::Error:
            {
                m_error.std::string::~string();
                break;
            }
            
            default: break;
        }

        m_state = EState::Invalid;
        
    }

    template <typename Type>
    void Result<Type>::Set(const Type& value)
    {
        Clear();

        new (&m_result) Type(value);
        m_state = EState::Valid;
    }

    template <typename Type>
    void Result<Type>::Set(Type&& value)
    {
        Clear();

        new (&m_result) Type(std::move(value));
        m_state = EState::Valid;
    }

    template <typename Type>
    void Result<Type>::SetError(const char* errorMsg)
    {
        Clear();

        new (&m_error) std::string(errorMsg);
        m_state = EState::Error;
    }

    template <typename Type>
    void Result<Type>::SetError(const std::string& errorMsg)
    {
        Clear();

        new (&m_error) std::string(errorMsg);
        m_state = EState::Error;
    }

    template <typename Type>
    void Result<Type>::SetError(std::string&& errorMsg)
    {
        Clear();

        new (&m_error) std::string(std::move(errorMsg));
        m_state = EState::Error;
    }
}
