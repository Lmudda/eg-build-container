# otway_stm32

STM32 implementations of drivers used in conjunction with the Otway event handling framework

# Driver interfaces

# Driver helpers

The "helpers" is a set of low level abstractions for interrupts, RCC clocks and so one which make it simpler to write a peripheral driver which is reusable. The idea is that a peripheral driver can be more self-contained. The most common features of the helpers are  

- A function to enable the relevant RCC clock given only the index of the peripheral instance.
- A function to enable the relevant interrupt(s) given only the index of the peripheral instance.
- A callback abstraction to allow a driver to connect a method to the relevant ISR, given only the 
  index of the peripheral index.

Each type of peripheral has and index in the form of an enumeration of the instances which exist on the device. The value of each enumerator is the base address of the corresponding peripheral instance. This 
ensures uniqueness and makes it very simple to recover pointer access to the registers at runtime by
casting the value to the relevant CMSIS struct type. Using enumerators also has the advantage of allowing
using to refer to peripheral instances with compiler-time constants, which can be exploited to perform 
compile-time checks on such things as pin selections.

TODO_AC Extend xxxHelpers::irq_enable() to include interrupt priority.

# DigitalInput

A DigitalInput represents in single input pin such as might be used for a button or sensor interrupt line. The class emits a Signal<bool> when the input value change. This can be immediately upon interrupt, or debounced through a software timer.

The class relies on EXTI interrupts so it is important that each instances uses a different pin index. Need to add an implementation which polls the input instead using a software timer - much less efficient.

# DigitalOutput

TODO_AC Add a pin allocation monitor to check for clashes during initialisation.

# SPIDriver

# SPIDriverDMA

TODO_AC Add an allocator for DMA streams so we don't have to choose indices. Should we use both DMA blocks?

# UARTDriver

# UARTDriverDMA

# UARTDriverUSB

TODO_AC Make sure the G4, U5 and F4 implementations are up to date / in sync. The F4 could be removed.


