/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code
// is confidential information and must not be disclosed to third parties or used without
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#include "signals/Signal.h"
#include "utilities/MemoryPool.h"
#include "utilities/ErrorHandler.h"
#include <cstring>
#include "utilities/Unreachable.h"

// If OTWAY_MAX_SIGNAL_LINKS is defined then use it to initialise
// MAX_SIGNAL_LINKS. Otherwise use a default of 200 links.
#if defined(OTWAY_MAX_SIGNAL_LINKS)
static constexpr uint16_t MAX_SIGNAL_LINKS = OTWAY_MAX_SIGNAL_LINKS;
#else
static constexpr uint16_t MAX_SIGNAL_LINKS = 200;
#endif


namespace eg {


// Globally shared pool for all link objects used in signals.
// Could be a static member of the Signal class, but this just exposes more of the implementation.
// Not that we care much about this for a template... Could replace this with heap allocation when
// using FreeRTOS... Can shrink the size of this pool to match the application when it is complete.
static MemoryPool<DummyLink, MAX_SIGNAL_LINKS> g_signal_pool;


uint16_t SignalBase::pool_size()
{
    return MAX_SIGNAL_LINKS;
}


uint16_t SignalBase::pool_free()
{
    return g_signal_pool.available();
}


SignalBase::~SignalBase()
{
    // Walk the collection of connected callbacks, unhook them all, and free them.
    CriticalSection cs;
    while (m_head)
    {
        DummyLink* head = m_head;
        m_head = head->next_head;
        head->next_head = nullptr;
        while (head)
        {
            DummyLink* link = head;
            head = link->next_link;
            link->next_link = nullptr;
            free_link(link);
        }
    }
}


// Prepend the link to the the linked list of earlier connections.
void* SignalBase::connect(void* link_in)
{
    // We maintain a two dimensional linked list (a linked list of linked lists).
    // The main list has an item for each distinct emit function. Each sub list
    // has one or more connections using the same emit function.
    //
    //   m_head ----> next_head ----> 0
    //      |             |
    //      v             v
    //   next_link        0
    //      |
    //      v
    //      0

    DummyLink* link = reinterpret_cast<DummyLink*>(link_in);

    DummyLink* next = m_head;
    // Find the sublist, if any, with the correct emit function.
    while (next && (next->loop != link->loop))
    {
        next = next->next_head;
    }
    if (next)
    {
        // We found a sublist. Walk this and append the new link.
        while (next->next_link != 0)
        {
            next = next->next_link;
        }
        next->next_link = link;
        link->next_link = 0;
    }
    else
    {
        // We did not find a sublist. Prepend the new link as the head of
        // a new sublist.
        link->next_head = m_head;
        m_head = link;
    }

    return link;
}


bool SignalBase::disconnect(void* data)
{
    CriticalSection cs;

    bool result = false;

    // This is the link we want to remove.
    DummyLink* link = static_cast<DummyLink*>(data);

    // Snip out the sub list with the same event loop.
    DummyLink* curr_head = m_head;
    DummyLink* prev_head = 0;
    while (curr_head)
    {
        if (curr_head->loop == link->loop)
        {
            if (prev_head)
                prev_head->next_head = curr_head->next_head;
            else
                m_head = curr_head->next_head;
            break;
        }
        prev_head = curr_head;
        curr_head = curr_head->next_head;
    }

    // Snip out the link from the sub list - it might be the whole sublist.
    // curr_head is NULL if there was no matching sublist.
    DummyLink* curr_link = curr_head;
    DummyLink* prev_link = 0;
    while (curr_link)
    {
        if (curr_link == link)
        {
            result = true;
            if (prev_link)
                prev_link->next_link = curr_link->next_link;
            else
                curr_head = curr_link->next_link;
            break;
        }
        prev_link = curr_link;
        curr_link = curr_link->next_link;
    }

    // Re-insert the sublist.
    // curr_head is NULL if no sublist was found or if it contained only a single item.
    if (curr_head)
    {
        curr_head->next_head = m_head;
        m_head               = curr_head;
    }

    // Return the link object to the memory pool.
    if (result)
    {
        free_link(data);
    }

    return result;
}


// Called from the signal template to allocate a new link to store a new connection.
void* SignalBase::alloc_link()
{
    void* result = g_signal_pool.alloc();
    if (result)
    {
        // Need to ensure there are no remnants (dangling pointers) from previous uses. Could/should do this in the constructor,
        // but this code involves no repitition in template specializations.
        std::memset(result, 0, sizeof(DummyLink));
    }
    else
    {
        // This is just to catch the case where MAX_SIGNAL_LINKS is too small to supported all the
        // required signal connections in the application.
        Error_Handler(); // LCOV_EXCL_LINE
    }
    return result;
}


// Called from the signal template to free the link from a deleted connection.
void SignalBase::free_link(void* link)
{
    g_signal_pool.free(static_cast<DummyLink*>(link));
}


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


// // Copy constructor copies only as many bytes as are used in the data.
// Event::Event(const Event& other)
// : m_signal(other.m_signal)
// , m_length(other.m_length)
// {
//     std::memcpy(&m_data[0], &other.m_data[0], m_length);
// }


// // Assignment operator copies only as many bytes as are used in the data.
// Event& Event::operator=(const Event& other)
// {
//     m_signal = other.m_signal;
//     m_length = other.m_length;
//     std::memcpy(&m_data[0], &other.m_data[0], m_length);
//     return *this;
// }


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
