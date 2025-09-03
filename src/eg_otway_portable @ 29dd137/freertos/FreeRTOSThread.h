/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "utilities/NonCopyable.h"
#include "utilities/ErrorHandler.h"


namespace eg {


// Simple wrapper for a CMSIS OS thread, which is itself a thin wrapper for a FreeRTOS thread.
// CMSIS OS is an ARM abstraction which it might be simpler just to bypass. The purpose of this
// class is to encapsulate the control block and stack for the thread so we can more cleanly 
// create statically allocated threads. The class is abstract: derivatives implement execute(),
// which is called in the thread context.
template <uint32_t StackSizeBytes>
class FreeRTOSThread : public NonCopyable
{
    static_assert(configSUPPORT_STATIC_ALLOCATION == 1, "We are using static allocation for threads");
    static constexpr uint32_t StackSizeDWords = (StackSizeBytes + sizeof(uint32_t) - 1) / sizeof(uint32_t);

    public:
        FreeRTOSThread(const char* thread_name, osPriority_t priority)
        {
            osThreadAttr_t attr{}; 
            attr.name          = thread_name,
            // osThreadDetached (default) or osThreadJoinable   
            attr.attr_bits     = osThreadDetached;   
            attr.cb_mem        = &m_control;
            attr.cb_size       = sizeof(m_control);
            attr.stack_mem     = &m_stack[0];
            attr.stack_size    = sizeof(uint32_t) * StackSizeDWords; 
            attr.priority      = priority; //osPriorityNormal;
            // We don't care about these.
            //attr.tz_module; 
            //attr.reserved;   

            // Calls xTaskCreateStatic because cb_mem and stack_mem are non-zero 
            m_id = osThreadNew(&FreeRTOSThread::execute_func, this, &attr);
            if (!m_id) 
            {
                Error_Handler(); // LCOV_EXCL_LINE
            }
        }

        osThreadId_t id() const { return m_id; }                   
       
    private:
        // This runs the event loop or whatever. Must inherit to provide this.
        virtual void execute() = 0;
        // Static function needed because C API.
        static void execute_func(void* arg);

    private:
        // The stack is required to be 8-byte aligned for some reason.
        // The control block needs to be only 4-byte aligned.
        alignas(uint32_t) StaticTask_t m_control{};
        alignas(uint64_t) uint32_t     m_stack[StackSizeDWords];
        osThreadId_t                   m_id{};                   
};


template <uint32_t StackSizeBytes>
void FreeRTOSThread<StackSizeBytes>::execute_func(void* arg)
{
    FreeRTOSThread* self = static_cast<FreeRTOSThread*>(arg);
    self->execute();
}


} // namespace eg {


