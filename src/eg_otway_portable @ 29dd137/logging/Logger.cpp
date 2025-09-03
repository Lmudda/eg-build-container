/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "logging/Logger.h"
#include "utilities/CriticalSection.h"
#include "utilities/ErrorHandler.h"
#include <cstdio>
#include <cstring>
#include "utilities/Unreachable.h"

namespace eg {


void Logger::raw(const char* format, ...)
{
    CriticalSection cs;

    va_list args;
    va_start(args, format);
    int result = vsnprintf(m_buffer, kBufferSize, format, args);
    if (result < 0) // On encode error. TODO: consider action if the return is > kBufferSize (i.e., buffer too small)
    {
        Error_Handler(); EG_UNREACHABLE //LCOV_EXCL_LINE
    }
    else 
    {
        write(m_buffer, true);
        va_end(args);
    }
    
}


void Logger::write(Level level, const char* file, long line, const char* function, const char* format, va_list args)
{
    write(level_name(level), file, line, function, format, args);
} 


void Logger::write(const char* level_name, const char* file, long line, const char* function, const char* format, va_list args)
{
    CriticalSection cs;

    // Log level - leave some space in the buffer for a final \n and null.
    int buflen = kBufferSize - 1;
    int length = 0;
    int result = 0;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Log level prefix. 
    // This is controlled internally and won't fail at all unless the buffer is too small.
    result = snprintf(m_buffer, buflen, "%s", level_name);
    // Skip check on the value of result - performance - premature optimisation?
    // if (result < 0)
    // {
    //     write("Level encoding error", true);
    //     return;
    // }
    // length = std::min(length + result, buflen - 1);
    length += result;

    // These callbacks could probably be made into compile time constants, in which case 
    // the code could use if constexpr to make the code a little more efficient.

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Time stamp prefix.
    // This is controlled internally and won't fail at all unless the buffer is too small.
    if (m_get_tick_func != nullptr)
    {
        // This doesn't include the date and will wrap after UINT32_MAX ms.
        uint32_t ticks = m_get_tick_func();  
        uint16_t millis = ticks % 1000;
        uint16_t secs   = (ticks / 1'000) % 60;      
        uint16_t mins   = (ticks / 60'000) % 60;      
        uint16_t hours  = (ticks / 3'600'000);      

        result = snprintf(m_buffer + length, buflen - length, "[%02u:%02u:%02u.%03u] ", hours, mins, secs, millis);
        // Skip check on the value of result - performance - premature optimisation?
        // if (result < 0)
        // {
        //     write("Tick time encoding error", true);
        //     return;
        // }
        // length = std::min(length + result, buflen - 1);
        length += result;
    }
    // Time stamp - preferred implementation - can easily obtain time from RTC or tick counter.
    else if (m_get_datetime_func)
    {
        // Could have an option to omit the date portion.
        DateTime dt{};
        m_get_datetime_func(dt);
        result = snprintf(m_buffer + length, buflen - length, "[%04u/%02u/%02u %02u:%02u:%02u.%03u] ", 
            dt.year, dt.month,  dt.day, 
            dt.hour, dt.minute, dt.second, dt.millis);
        // Skip check on the value of result - performance - premature optimisation?
        // if (result < 0)
        // {
        //     write("Time stamp encoding error", true);
        //     return;
        // }
        // length = std::min(length + result, buflen - 1);
        length += result;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // File location prefix.
    // This is controlled internally and won't fail at all unless the buffer is too small.
    // This **might** happen if the file name and/or function name are absurdly long.  
    // Remove the path from __FILE__ macro value. Can we find a compile-time solution for this?
    // There is a neat trick to have CMake put __FILE_NAME__ on the command line (so this calculation 
    // is done at compile time) but I could not get it to work.
    // https://stackoverflow.com/questions/8487986/file-macro-shows-full-path.
    const char* file_name = std::strrchr(file, '/');
    file_name = file_name ? (file_name + 1) : file;

    if (function != nullptr)
    {
        result = snprintf(m_buffer + length, buflen - length, "%s:%li (%s) ", file_name, line, function);
    }
    else
    {
        result = snprintf(m_buffer + length, buflen - length, "%s:%li ", file_name, line);
    }

    // Skip check on the value of result - performance - premature optimisation?
    // if (result < 0)
    // {
    //     write("Location encoding error", true);
    //     return;
    // }
    // Maybe keep this in case the buffer is too short.
    // length = std::min(length + result, buflen - 1);
    length += result;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Formatted message.
    // This could suffer from either and encoding error or a short buffer. The worst that can happen
    // with a short buffer is a truncated message. Encoding errors are unlikely, but might result from
    // typos in the format string.
    result = vsnprintf(m_buffer + length, buflen - length, format, args);
    if (result < 0)
    {
        write("Format encoding error", true);
        return;
    }
    length += result;

    // Terminate the formatted message with ellipsis to indicate that it was truncated by snprintf().
    if (length > (buflen - 1))
    {
        m_buffer[buflen - 2] = '.';
        m_buffer[buflen - 3] = '.';
        m_buffer[buflen - 4] = '.';
        length = buflen - 1;
    }

    // Append a new line to ensure the UART backend started a new line for each message. 
    // TODO_AC Remove this and let the backend deal with it?
    m_buffer[length]     = '\n';
    m_buffer[length + 1] = 0;
    write(m_buffer, true);
} 


void Logger::write(const char* message, bool new_line)
{
    for (uint8_t i = 0; i < kMaxBackends; ++i)
    {
        if (m_backends[i] != nullptr)
        {
            m_backends[i]->write(message, new_line);
        } 
    }
}


const char* Logger::level_name(Level level)
{
    switch (level)
    {
        case Level::Assert: return "FATAL: "; 
        case Level::Error:  return "ERROR: "; 
        case Level::Warn:   return "WARN:  ";
        case Level::Info:   return "INFO:  ";
        case Level::Trace:  return "TRACE: "; 
        case Level::Debug:  return "DEBUG: ";
        default: EG_UNREACHABLE //LCOV_EXCL_LINE
    }
}


void Logger::register_ticker(GetTickFunc get_tick_func)
{
    CriticalSection cs;
    m_get_tick_func = get_tick_func;
}


void Logger::register_datetime_source(GetDateTimeFunc get_datetime_func)
{
    CriticalSection cs;
    m_get_datetime_func = get_datetime_func;
}


bool Logger::register_backend(ILoggerBackend& backend)
{
    CriticalSection cs;

    // Check if already registered.
    for (uint8_t i = 0; i < kMaxBackends; ++i)
    {
        if (m_backends[i] == &backend) 
        {
            return true;
        }
    }

    // Find a slot for the backend pointer.
    for (uint8_t i = 0; i < kMaxBackends; ++i)
    {
        if (m_backends[i] == nullptr)
        {
            m_backends[i] = &backend;
            return true;
        } 
    }

    // No room to store the backend pointer.
    return false;
}


} // namespace eg {    
