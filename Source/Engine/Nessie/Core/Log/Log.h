#pragma once
// Log.h
#include "Logger.h"

#undef NES_LOGGING_ENABLED
#ifndef NES_RELEASE
#define NES_LOGGING_ENABLED 1
#else
#define NES_LOGGING_ENABLED 0
#endif

//-----------------------------------------------------------------------------------------------------------------------------
//		NOTES:
//      To make the File Name pretty for Debugging Messages:
//      Adapted from: https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
//           - strrchr() gets the last occurrence of a certain character in a const char*.
//           - It either returns a pointer to the spot where it finishes, or it returns nullptr if nothing is found.
//           - So, we either return the pointer to the path just after the final '\' OR we return the file path.
//		
///		@brief : Get just the filename and extension from the __FILE__ macro.
//-----------------------------------------------------------------------------------------------------------------------------
#define GET_CURRENT_FILENAME (strrchr(__FILE__, '\\')? strrchr(__FILE__, '\\') + 1 : __FILE__)

#if NES_LOGGING_ENABLED
#define NES_INIT_LOGGER(LogOutputFilepath) nes::Logger::Init(LogOutputFilepath)
#define NES_CLOSE_LOGGER() nes::Logger::Close()
#define NES_LOAD_LOG_CATEGORIES(CategoryDataFilepath) nes::Logger::LoadCategories(CategoryDataFilepath)

#define NES_LOG(...) nes::Logger::QuickLog(nes::LogSeverity::kLog, __VA_ARGS__)
#define NES_WARN(...) nes::Logger::QuickLog(nes::LogSeverity::kWarning, __VA_ARGS__)
#define NES_ERROR(...) nes::Logger::QuickLog(nes::LogSeverity::kError, __VA_ARGS__)
#define NES_CRITICAL(...) nes::Logger::QuickLog(nes::LogSeverity::kCritical, __VA_ARGS__); std::abort()

#define NES_LOGV(Category, ...) nes::Logger::Log(Category, __VA_ARGS__)
#define NES_WARNV(Category, ...) nes::Logger::VitalLog(Category, nes::LogSeverity::kWarning, __VA_ARGS__)
#define NES_ERRORV(Category, ...) nes::Logger::VitalLog(Category,  nes::LogSeverity::kError, __VA_ARGS__)
#define NES_CRITICALV(Category, ...) nes::Logger::VitalLog(Category,  nes::LogSeverity::kCritical, __VA_ARGS__); std::abort()
#else

#define NES_INIT_LOGGER(LogOutputFilepath) void(0)
#define NES_CLOSE_LOGGER() void(0)
#define NES_LOAD_LOG_CATEGORIES(CategoryDataFilepath) void(0)

#define NES_LOG(...) void(0)
#define NES_WARN(...) void(0)
#define NES_ERROR(...) void(0)
#define NES_CRITICAL(...) std::abort()

#define NES_LOGV(Category, ...) void(0)
#define NES_WARNV(Category, ...) void(0)
#define NES_ERRORV(Category, ...) void(0)
#define NES_CRITICALV(Category, ...) std::abort()

#endif