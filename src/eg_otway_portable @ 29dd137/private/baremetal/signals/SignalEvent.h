/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once 
#include "SignalBase.h"
#include <cstring>
#include <new>
#include <type_traits>
#include "utilities/ErrorHandler.h"
#include "utilities/Unreachable.h"


namespace eg {


// This class is used to hold different types of signal and the associated argument data 
// anonymously, so that the scheduler can store a queue of pending signals in a homogeneous
// list (basically a simple array).
class Event
{
public:
    static constexpr uint16_t kMaxEventData = 64;

public:
    // Normal constructor used when emitting emits to the scheduler.
    Event(const SignalBase& signal);
    // Default constructor is required in the Scheduler.
    Event();

    // NOTE: These methods make Event not trivially copyable. We copy these objects
    // in and out of a FreeRTOS queue, which probably uses memcpy. That assumes trivially 
    // copyable. Probably not an issue in this case, but better to fix it.
    // // Copy constructor should reduce the amount of copying by only copying the 
    // // number of data bytes which are meaningful.
    // Event(const Event& other);
    // // Assignment operator should reduce the amount of copying by only copying the 
    // // number of data bytes which are meaningful. 
    // Event& operator=(const Event& other);

    ///Pack data into an event
    ///Copy constructs the data into the event
    template<typename T>
    void pack(const T& val)
    {
        // The size test is insufficient for more than one argument.
        static_assert(std::is_trivially_copyable_v<T> == true);
        static_assert(sizeof(T) <= kMaxEventData);
        // Why does this suddenly cause a hardfault?
        //new (reinterpret_cast<T*>(m_data)) T(val);
        // Only allowed for trivially copyable types.
        std::memcpy(&m_data[m_length], &val, sizeof(T));
        m_length += sizeof(T);
    }

    ///Unpack data from the event
    template<typename T>
    void unpack(T& val, uint16_t offset = 0) const
    {
        std::memcpy(&val, &m_data[offset], sizeof(T));
    }

    // This is called by the scheduler when it is time to dispatch the signals.
    void dispatch() const 
    {   
        if (m_signal)
        {
            m_signal->dispatch(*this); 
        }
        else
        {
            Error_Handler(); EG_UNREACHABLE //LCOV_EXCL_LINE
        }
    }

public:
    const SignalBase* m_signal{};
    uint16_t          m_length{};
    uint8_t           m_data[kMaxEventData] = {};
};


// This is important because objects are likely to be copied with memcpy.
// This does mean redundantly copying unused part of m_data, but that's how 
// the cookie crumbles. It was only a minor optimisation to prevent that.
// Could also make kMaxEventData smaller.
static_assert(std::is_trivially_copyable_v<Event>);


} // namespace eg {

