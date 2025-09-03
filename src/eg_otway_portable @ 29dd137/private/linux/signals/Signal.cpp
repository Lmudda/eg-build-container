/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "signals/Signal.h"
#include <cstring>
#include "utilities/ErrorHandler.h"
#include "utilities/Unreachable.h"


namespace eg {


// Normal constructor
Event::Event(const SignalBase& signal)
: m_signal(&signal)
, m_length(0U)
{
}


// Default constructor - needed for the scheduler
Event::Event()
: m_signal(0)
, m_length(0U)
{
}


// Copy constructor copies only as many bytes as are used in the data.
Event::Event(const Event& other)
: m_signal(other.m_signal)
, m_length(other.m_length)
{
    std::memcpy(&m_data[0], &other.m_data[0], m_length);
}


// Assignment operator copies only as many bytes as are used in the data.
Event& Event::operator=(const Event& other)
{
    m_signal = other.m_signal;
    m_length = other.m_length;
    std::memcpy(&m_data[0], &other.m_data[0], m_length);
    return *this;
}


IEventLoop& this_event_loop()
{
    IEventLoop* loop = this_event_loop_impl();
    if (!loop)
    {
        Error_Handler(); EG_UNREACHABLE //LCOV_EXCL_LINE
    }
    return *loop;
}


IEventLoop& default_event_loop()
{
    IEventLoop* loop = default_event_loop_impl();
    if (!loop)
    {
        Error_Handler(); EG_UNREACHABLE //LCOV_EXCL_LINE
    }
    return *loop;
}


} // namespace eg {


