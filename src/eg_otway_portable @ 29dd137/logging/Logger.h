/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "logging/ILoggerBackend.h"
#include "utilities/NonCopyable.h"
#include <array>
#include <cstdint>
#include <cstdarg>


#if !defined(OTWAY_LOGGER_BUFFER_SIZE)
#define OTWAY_LOGGER_BUFFER_SIZE 256U
#endif 

#if !defined(OTWAY_LOGGER_MAX_BACKENDS)
#define OTWAY_LOGGER_MAX_BACKENDS 4U
#endif 

#if defined(_MSC_VER)
#define FORMAT_ATTRIBUTE_CHECK(string_index, first_to_check)
#else
#define FORMAT_ATTRIBUTE_CHECK(string_index, first_to_check) __attribute__((format(printf, (string_index), (first_to_check))))
#endif


namespace eg {

// A basic logger with a few customisation points:
// - Which log levels are ignored (compile time).
// - Optional user definable backends to output the formatted messages (runtime).
// - Optional user definable callbacks to provide time stamps when formatting the messages.
// The macros EG_LOG_XXXXX below capture the file/line/function context of each log message.
class Logger : public NonCopyable
{
public:
    // Smaller values are higher priorities. Setting MaxLevel to a higher value results
    // in more verbose logging due to lower priority messages. Setting MaxLevel to 0
    // disables all logging except for Assert errors. TODO_AC We possibly want to enable 
    // these features independently (use a bitfield), or just enable Debug separately.
    enum class Level : uint8_t 
    { 
        // Values explicit here for clarity with macros.
        Assert = 0, // A condition so bad that the application cannot continue.
        Error  = 1, // A condition that definitely shouldn't happen but the application can continue.
        Warn   = 2, // A condition that probably shouldn't happen but the application is essentialy unaffected.
        Info   = 3, // Confirmatory messages about the normal operation of the application.
        Debug  = 4, // Low level messages used mostly during development.
        Trace  = 5, // Stupidly detailed low level messages used during development. 
    };

    // This constant is passed in from the build systems via a #define set in the top level CMakeLists.txt.
    static constexpr Level kMaxLevel = static_cast<Level>(OTWAY_MAX_LOG_LEVEL);

    // This is used in combination with register_datetime_source() to provide the current 
    // time to the logger so each message can have a formatted timestamp. 
    struct DateTime
    {
        // uint16_t to avoid casting in the snprintf call which formats the timestamp.
        uint16_t year;
        uint16_t month;
        uint16_t day;
        uint16_t hour;
        uint16_t minute;
        uint16_t second;
        uint16_t millis;
    };

public:
    // Pass a pointer to a timer function (assumed to be milliseconds) if you want the 
    // time preprended to messages. TODO_AC Deprecated - prefer register_datetime_source()
    using GetTickFunc = uint32_t (*)();
    static void register_ticker(GetTickFunc get_tick_func);

    // This is a slightly better abstraction. We can source the time stamp directly 
    // from an RTC, if we have one, or write a function to convert the current tick count
    // into a DateTime. 
    using GetDateTimeFunc = void (*)(DateTime&);
    static void register_datetime_source(GetDateTimeFunc get_datetime_func);

    // The logger doesn't know where it's output goes. The backend might write to a UART, 
    // a file, or something else.     
    static bool register_backend(ILoggerBackend& backend);

    // Make this a template to avoid code duplication.
    template <Level LEVEL>
    static void log(const char* file, long line, const char* function, const char* format, ...) FORMAT_ATTRIBUTE_CHECK(4, 5);

    // {
    //     va_list args;
    //     va_start(args, format);
    //     vlog<LEVEL>(file, line, function, format, args);
    //     va_end(args);
    // }

    // Directly formatted output without timestamp and whatnot.
    static void raw(const char* format, ...) FORMAT_ATTRIBUTE_CHECK(1, 2);
private:
    template <Level LEVEL>
    static void vlog(const char* file, long line, const char* function, const char* format, va_list args) FORMAT_ATTRIBUTE_CHECK(4, 0);
    // {
    //     // Belt and braces. Optimiser would probably remove empty functions 
    //     // due to this condition, but the macros below to define nothing for 
    //     // excluded levels in any case.
    //     if constexpr (kMaxLevel >= LEVEL)
    //     {
    //         write(LEVEL, file, line, function, format, args);
    //     }
    // }

    static void write(Level level, const char* file, long line, const char* function, const char* format, va_list args) FORMAT_ATTRIBUTE_CHECK(5, 0); 
    static void write(const char* level_name, const char* file, long line, const char* function, const char* format, va_list args) FORMAT_ATTRIBUTE_CHECK(5, 0);
    static void write(const char* message, bool new_line);
    static const char* level_name(Level level);

private:
    // Could use a vector for Linux but how many backends do we realistically need?
    // Could specify the size through a CMake definition but how likely is it needed?
    static constexpr uint8_t kMaxBackends = OTWAY_LOGGER_MAX_BACKENDS;
    static_assert(kMaxBackends >= 1, "Logger needs at least one backend");
    inline static std::array<ILoggerBackend*, kMaxBackends> m_backends{};
    
    // Callbacks to obtain times for formatted timestamps.
    inline static GetTickFunc     m_get_tick_func{};
    inline static GetDateTimeFunc m_get_datetime_func{};

    // Shared buffer used to format messages. A critical section is used to serialise calls 
    // to avoid messages interrupting each other. A stack buffer would be better but is too 
    // large for embedded. Could use a pool of buffers or thread-local-storage to support 
    // concurrent messages.
    static constexpr uint16_t kBufferSize = OTWAY_LOGGER_BUFFER_SIZE;
    static_assert(kBufferSize >= 256, "Logger needs at least a 256 byte buffer");
    inline static char m_buffer[kBufferSize];
};


template <Logger::Level LEVEL>
void Logger::log(const char* file, long line, const char* function, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vlog<LEVEL>(file, line, function, format, args);
    va_end(args);
}


template <Logger::Level LEVEL>
void Logger::vlog(const char* file, long line, const char* function, const char* format, va_list args) 
{
    // Belt and braces. Optimiser would probably remove empty functions 
    // due to this condition, but the macros below to define nothing for 
    // excluded levels in any case.
    if constexpr (kMaxLevel >= LEVEL)
    {
        write(LEVEL, file, line, function, format, args);
    }
}


} // namespace eg {    


// There's a little trick used to take absolute paths out of the __FILE__ macro used for logging at 
// compile time. The following code is used to modify the command line for each translation unit:
// set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D__FILENAME__='\"$(basename $(notdir $(abspath $@)))\"'")


#if (OTWAY_MAX_LOG_LEVEL >= 1)
#define EG_LOG_ERROR(...)  ::eg::Logger::log<::eg::Logger::Level::Error>(__FILE__, __LINE__, __func__, __VA_ARGS__)
//#define EG_LOG_ERROR(...)  ::eg::Logger::log<::eg::Logger::Level::Error>(__FILENAME__, __LINE__, __func__, __VA_ARGS__)
#else
#define EG_LOG_ERROR(...)
#endif

#if (OTWAY_MAX_LOG_LEVEL >= 2)
#define EG_LOG_WARN(...)   ::eg::Logger::log<::eg::Logger::Level::Warn>(__FILE__, __LINE__, __func__, __VA_ARGS__)
//#define EG_LOG_WARN(...)   ::eg::Logger::log<::eg::Logger::Level::Warn>(__FILENAME__, __LINE__, __func__, __VA_ARGS__)
#else
#define EG_LOG_WARN(...)
#endif

#if (OTWAY_MAX_LOG_LEVEL >= 3)
#define EG_LOG_INFO(...)   ::eg::Logger::log<::eg::Logger::Level::Info>(__FILE__, __LINE__, __func__, __VA_ARGS__)
//#define EG_LOG_INFO(...)   ::eg::Logger::log<::eg::Logger::Level::Info>(__FILENAME__, __LINE__, __func__, __VA_ARGS__)
#else
#define EG_LOG_INFO(...)
#endif

#if (OTWAY_MAX_LOG_LEVEL >= 4)
#define EG_LOG_DEBUG(...)  ::eg::Logger::log<::eg::Logger::Level::Debug>(__FILE__, __LINE__, __func__, __VA_ARGS__)
//#define EG_LOG_DEBUG(...)  ::eg::Logger::log<::eg::Logger::Level::Debug>(__FILENAME__, __LINE__, __func__, __VA_ARGS__)
#else
#define EG_LOG_DEBUG(...)
#endif

#if (OTWAY_MAX_LOG_LEVEL >= 5)
#define EG_LOG_TRACE(...)  ::eg::Logger::log<::eg::Logger::Level::Trace>(__FILE__, __LINE__, __func__, __VA_ARGS__)
//#define EG_LOG_TRACE(...)  ::eg::Logger::log<::eg::Logger::Level::Trace>(__FILENAME__, __LINE__, __func__, __VA_ARGS__)
#else
#define EG_LOG_TRACE(...)
#endif

// This is not strictly necessary but seems like good form.
#if (OTWAY_MAX_LOG_LEVEL >= 6)
#error Invalid value for OTWAY_MAX_LOG_LEVEL
#endif 

#define EG_LOG_RAW(...)    ::eg::Logger::raw(__VA_ARGS__)

