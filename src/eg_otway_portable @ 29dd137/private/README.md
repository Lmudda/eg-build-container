The private folder is not part of the Otway Portably library, but holds implementation details. In particular, it holds platform-specific versions of some of the core library features such as the Signal class.

TODO_AC Otway portable: FreeRTOS support (Queue, EventLoop, Timers) - Signal OK probably
TODO_AC Otway portable: Timer implementations for Linux and FreeRTOS
TODO_AC Otway portable: Have single EventLoop API to replace BareMetalEventLoop and ThreadEventLoop
TODO_AC Otway portable: Unified CriticalSection/DisableInterrupts to be used for FreeRTOS and bare-metal. 
        Without breaking Linux dependencies. Better to use only in platform specific code?
TODO_AC Otway portable: Can we make Error_Handler() only available for embedded code?