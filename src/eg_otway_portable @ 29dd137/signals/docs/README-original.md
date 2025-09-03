# Otway asynchronous event handling framework

Otway is an abstraction layer intended to make asynchronous event handling in bare-metal and RTOS applications easy to implement and understand. 

- Completely replaces the need to tick all the application objects round-robin (a superloop). 
- Completely replaces the need for application objects to individually maintain one or more collections of callbacks which they iterate over to invoke them. 
- Allows callbacks to be invoked either synchronously (direct calls) or asynchronously (via an event loop). Asynchronous is generally most useful.
- Easily extended to multiple threads, generally with different priorities, which each run their own event loop. [I have used this to perform 200Hz sensor readings in the foreground while running a 100ms calculation at 1Hz in the background.]

Overall, the design is intended to help decouple the sources/producers of events from the sinks/consumers of those events.

The two key elements of the frame are **event loops** and **signals**:

## Using signals

Suppose we have two classes **Producer** and **Consumer**. **Producer** might for example be driver (such as a UART) which emits an event from an ISR or another event handler. **Consumer** needs to be notified whenever new events are emitted by **Producer** (it might be a packet finder which searches the incoming byte stream from the UART for valid packets).

The producer owns one or more **Signal** objects and exposes them publicly to allow consumers to make connections:

```c++
class Producer
{
public:
    // Provide access to the signal - the proxy just limits 
    // the API to connection methods - not strictly necessary.
    eg::SignalProxy<uint8_t> on_rx_byte() 
    {
        return eg::SignalProxy{m_on_rx_byte};
    }   

private:
    // This called to emit an event, perhaps from an ISR. 
    void emit_event(uint8_t value)
    {
        // This is basically a deferred callback to whichever 
        // methods have been connected by consumers of this event.
        m_on_rx_byte.emit(value);
    }    

private:
    eg::Signal<uint8_t> m_on_rx_byte;
};
```

The consumer needs some visibility of the producer API in order to 
make connections to its signal(s). A consumer can connect functions to signals in multiple producers:

```c++
class Consumer
{
public:
    Consumer(Producer& producer)
    {
        // Connect to the producer's signal to receive events when 
        // they are emitted.
        producer.on_rx_byte().connect<&Producer::handle_on_rx_byte>(this);
    }

private:
    // This is called asynchronously shortly after the producer 
    // calls Signal::emit().
    void handle_on_rx_byte(const uint8_t& value)
    {
        // Do something or nothing or whatever.
    }
};
```

Trivial application using this example code:
```c++
int main()
{
    Producer producer;
    Consumer consumer1{producer};
    Consumer consumer2{producer};
    ...
}
```

The application is also responsible for creating one or more event loops (at most one event loop per thread). There is a little bit of boiler plate to integrate the event loop implementations with signals. This serves to tell the **Signal** class where to queue events when they are emitted, and which connected functions should be called in a given thread when dispatching events.

## Event loops

An event loop is little more than a queue of pending **events** which are each dispatched and run to completion sequentially in the ordered in which they occur. Event producers place events into the queue, and event consumers receive the events when it is their turn to be dispatched by the queue. This is enough to introduce asynchronicity, and to safely marshal events from ISRs to the application, and between different threads within the application. The only requirement is for the event queue to be thread-safe/interrupt-safe.

## Signals

A signal is basically an asynchronous implementation of the Observer pattern for a specific callback signature. The name was inspired by my past work with Qt Signals and Slots. Each signal maintains a collection of connected callbacks. Event consumers register their interest in events from a signal by calling a method to connect a callback to it.

The classic Observer pattern is synchronous: invoking a call on the signal's API immediately forwards the call to all of the connected callbacks, passing any arguments for the call to each one in turn. 

In our case, the signal collaborates with event queues. Invoking a call on the signal's API causes it to place an event into an event loop's queue of pending events (actually zero or more event loops). Then, a little later, in the context in which the event loop is running, the event is sent back to the signal, at which point it now calls the connected callbacks.

# Implementation

Event loops are now implementations of `IEventLoop`, which replaces the more clunky `EmitFunc` describes below.

## class Event

An `Event` contains a buffer to hold the packed arguments (if any) of the associated callback function invocation. It also contains a pointer to the Signal object which created it. In some ways an `Event`resembles a command object in the **Command** pattern: it essentially encapsulates a deferred callback.

The `Event` API includes the following methods:

- `pack<Arg>(const Arg& arg)`: called by `Signal<Arg>::emit(const Arg&)` to pack the value of arg into the event before passing it to an `EventLoop`.
- `unpack<Arg>(Arg& arg)`: called by `Signal<Arg>::dispatch(const Event&)` to unpack the the value of arg before finally calling the callback(s).
- `dispatch()`: called by `EventLoop::run()` - this just forwards the call to the signal which created the event in the first place: `Signal::dispatch(*this)`.

## class EventLoop

An `EventLoop` contains a `RingBuffer<Event>` which it used for the queue of pending events. For a system using an RTOS, it makes sense to replace the ring buffer with an RTOS queue structure because it is thread safe and can be used to block.

The `EventLoop` API includes the following methods:

- `post(const Event&)`: pushes a new Event onto the queue of pending events. This is called (indirectly) by `Signal::emit()`.
- `run()`: takes the front event from the queue (if any) and calls `Event::dispatch()`, and loops. This replaces the super loop that is seen in many embedded programs. For a system using an RTOS, we can block when the queue is empty. When all threads are blocked, we can think about entering a low power mode for the device. 

## class Signal<Arg>

A `Signal` maintains a data structure of connected callbacks. The structure is in principle something like `std::map<EmitFunc, std::list<CallbackFunc>>`. 

- An `EmitFunc` is a pointer to a free function which can post an event to a particular `EventLoop`'s queue. This is kind of a proxy for both the thread ID and the event loop instance. The indirection is intended to make signals independent of the particular RTOS or whatever we are using.
- A `CallbackFunc` is something like a super simple version of `std::function`, but which does not use the heap for storage. It basically boils down to a pointer to a function plus a pointer to an object. This is sufficient to use a free function, a static member function or a non-static member function as a callback. But not a lambda (in this implementation). 

[Lambda expressions are possible but require additional storage space. Essentially all use cases for lambdas with capture would be covered by `[this](const Arg& arg) { ... }` anyway (just capturing `this`), which can be achieved with a pointer to a member function and a pointer to an object. It *may* be true that using such a lambda would save some flash space used to pass pointers to member functions as template arguments, but that is for another day.]

The `Signal<Arg>` API includes the following methods:

- `connect<Class::MemberFuncPtr>(Class* self, EmitFunc emitter = default_emitter)`: this is called by event consumers to register their interest in events from a given signal object. The callback is a non-static member function of `Class`, which may be private. Connections are typically made in the consumer's constructor. The emitter argument tells the signal in which `EventLoop` the callback should be called. [The event may be emitted in this thread, a different thread, or an ISR.] 
- `connect<CallbackFuncPtr>(EmitFunc emitter = default_emitter)`: this is called by event consumers to register their interest in events from a given signal object. The callback is a static member function of some class, which may be private, or a free function.
- `disconnect(void*)`: the `connect()` methods return a pointer which can be used later to disconnect a callback. This is almost never useful for embedded applications because connections are typically made during initialisation and then live forever.
- `emit(const Arg& arg)`: This is called by an event producer (which typically owns one or more `Signal` objects as private members) to place an event into any `EventLoop` queues associated with connected callbacks.
- `call(const Arg& arg)`: this bypasses the event loop(s) and directly calls all connected callbacks. This is basically the classic synchronous Obverser pattern. It can be useful sometimes, but producers more often call `emit()`. 
- `dispatch(const Event& ev)`: this is called by Event::dispatch() to forward itself to the signal which created it, which is best placed to actually deal with the event (because it knows the types of the arguments and the callbacks which need to be called). There is a sense in which `Signal` is both the source and the sink of all events. It is the core of the whole mechanism.

## Assorted shenanigans

Note that a default `EmitFunc` mechanism is provided because, at least for single-threaded usage, passing this argument is redundant.

Implementations can vary, but for multi-threaded usage it makes sense to use **Thread Local Storage** to associate each EmitFunc with a particular thread ID. This information is used in calls to `Signal::dispatch()` to determine which callbacks to call within the current thread.


# Lifetime concerns

The design is predicated on the assumption that applications typically set up their event loops, Signals and connections during initialisation and that they then last until the device is powered down. This works pretty well, but it is worth making a few more detailed comments about the lifetime relationships between the various components. There are four interacting elements at play: `EventLoop`s, `Event`s, `Signal`s and callbacks/connections.

**EventLoop:** An `EventLoop` must not go out of scope while there are any `Signal` objects which might post events to its queue. That is, any `Signal` objects with connections which will be invoked via that `EventLoop`. This is generally a non-issue: an application has a fixed set of `EventLoop` objects which are created during start up and never destroyed.

**Event:** An `Event` is a short lived object placed into an `EventLoop` queue by `Signal::emit()`. Its lifetime is not a problem, but the `Signal` which created it must not be destroyed until the `Event` has been dispatched (which involves sending the event back to the `Signal`). This is generally a non-issue because events are so short-lived, but there is a potential race condition. It is better to avoid having any `Signal` objects which go out of scope, especially as members of short-lived objects such as, say, a structure representing a comms packet.

**Signal:** As mentioned above a `Signal` must remain in scope to dispatch any events it has emitted which are still in flight (i.e. still held in one or more `EventLoop` queues).

**Connection/Callback:** There is no formal type for a connection as it is captured within the `Signal` object. However, the object which is the target of the callback should not go out of scope without first disconnecting itself from the relevant signal. `Signal::connect()` returns a handle (currently just a `void*`) which can be used for this by calling `Signal::disconnect`. The handle can be cached as a member of the target. 

while it is worth noting these as potential concerns, in practice embedded applications almost never need to have `EventLoop`s or `Signal`s with non-static lifetimes.

# Software timers

Software timers are a particularly useful example of using `Signal`s. A `Timer` object has a period (typically in milliseconds) and a `Signal<>` which emits events when the timer fires. For example, you can do this:

```c++
class Blinky
{
public:
    Blinky()
    {
        // Connect a callback to the timer object's Signal.
        m_timer.on_update().connect<&Blinky::on_timer>(this);
        // Start the timer running.
        m_timer.start();       
    }

private:
    // This will be called at 2Hz 
    void on_timer()
    {
        // Toggle a LED.
    }

private:
    using Type = Timer::Type;
    // Configure the timer to fire every 500ms == 2Hz
    SoftwwareTimer m_timer{500, Type::Repeating};       
};
```

The `Timer` API includes the following methods:

- `Timer(uint32_t period, Type type)`: The constructor sets the initial interval and timer type.
- `set_period(uint32_t period)`: Sets the length of the interval. The granularity is typically 1ms but depends on the implementation.
- `set_type(Type type)`: Sets the type of the timer: `OneShot` or `Repeating`.
- `start()`: Changes the timer's state to running. Its signal will emit an event after the interval period has elapsed. A repeating timer will emit an event again after the same interval until stopped.
- `stop()`: Changes the timer's state to idle. No events will be emitted.
- `emit()`: This is really an implementation detail but could in principle be called by application code to force a timer event to be emitted immediately.
- `on_update()`: This returns a proxy object which allows event consumers to connect callbacks to the timer's internal `Signal<>` object.
- `get_tick_count()`: This static function returns the number of ticks since the application started running. The granularity is typically 1ms but depends on the implementation.

## Timer implementation

The implementation involves a class called `TimerQueue`. This manages a priority queue of running timers (this is implemented as a double linked list). The remaining ticks on each running timer is differential, so that it is only necessary to decrement the ticks on the next timer to fire. 

Starting a timer amounts to inserting it into the linked list. Stopping a timer amounts to removing it from the linked list. 

`TimerQueue` is ticked from the **SysTick** interrupt handler or some other interrupt-driven ticker. On each tick, it decrements the remaining ticks on the timer at the head of the list. When the ticks reach zero, the timer's `emit()` function is called. The timer is removed from the list and, if its type is `Repeating`, it is re-inserted.

## Alternative timer implementations

1. It is possible to use a hardware timer peripheral whose period matches the remaining period for the timer at the head of the running timers list. This can greatly reduce the number of interrupts. The peripheral's period is changed on every interrupt to match the remaining period of the new head of the list, or disabled if there are no more running timers. Some care is needed to avoid timer drift, if that matters.

2. When using FreeRTOS, or similar, we can drop `TimerQueue` entirely and rely on whatever the RTOS does internally to manage running timers. The implementation of `Timer` is quite different (e.g. trampolining a callback from the RTOS into a member function of a particular timer), but the API is unchanged.



