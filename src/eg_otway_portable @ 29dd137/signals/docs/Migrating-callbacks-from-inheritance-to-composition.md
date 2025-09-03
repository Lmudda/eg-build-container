# Migrating callbacks from inheritance to composition

The existing code in other embedded software projects uses a synchronous version of the Observer pattern for event handling based around the inheritance of a listener interface for each type of event. While this works, inheritance is a very tight form of coupling for this sort of thing, and there are a couple of other issues with this design.

- There is a lot of code duplication, or near duplication, in the objects which produce events (i.e. call the callbacks). Each one must maintain a collection of registered callbacks, and must include a function to loop over this collection in order to call all the callbacks. 
- The callback functions are necessarily public members of the classes which consume events (i.e. whose callbacks are called).
- Inheritance is a tight form of coupling which can be avoided in this case. Worse, it may be necessary to implement multiple inheritance (generally of interfaces), and this can necessitate virtual inheritance, which always feels like a bad smell.
- The callback mechanism is entirely synchronous, which is not always (or perhaps not often) desirable when decoupling subsystems.

This document is intended to walk step-by-step to an alternative implementation of the Observer pattern which resolves these issues. The first step is to write some code representative of the current architecture. We'll then change it step-by-step
 
## Example of the existing architecture

Suppose we have a key pad which needs to inform a collection of "listeners" in the application every time a new key press is recognised. We would have something like the following interface:

```c++
class IKeyListener
{
public:
    virtual void OnKeyPress(KeyID key) = 0;
};
```

Any object which wishes to be notified of key press events first needs to implement this interface.

Now suppose we have a class representing the key pad, the source of events which will lead to calls of OnKeyPress:

```c++
class KeyPad
: public IKeyPad // An interface for mocking purposes
{
public:
    // Listeners call this (typically from their constructors) to register their interest 
    // in key press events.
    void RegisterKeyListener(IKeyListener* listener)
    {
        m_listeners.push_back(listener);
    }

private:
    // This is called from after we have detected a key press, perhaps from an ISR.
    // I'm not sure if we actually would call this from an ISR as that might not be safe.
    void OnKeyPress(KeyID key)
    {
        for (auto& listener: m_listeners)
            listener->OnKeyPress(key);
    }

private:
    // Hold a list or vector of registered callbacks.
    // We would likely not use std::list for embedded projects. 
    std::list<IKeyListener*> m_listeners;
};
```

Finally, let's have a user interface class which wishes to be notified of key press events: 

```c++
// This class represents the user interface and needs to receive notifications of key press events.
// It inherits IKeyListener and registers itself with the KeyPad in its constructor.
class UserInterface
: public IUserInterface  // An interface for mocking purposes
, public IKeyListener    // An interface for event handling purposes
{
public:
    UserInterface(KeyPad& keypad)
    {
        // We may or may not want to hold the reference to keypad in a member to, for example,
        // Unregister during destruction. This is rarely a concern in embedded systems.
        keypad.RegisterKeyListener(this);
    }

    // This method needs to be public.
    void OnKeyPress(KeyID key) override
    {
        // ...
    }
};
```

This all works just fine but there a few observations to make:

- `KeyPad` needs to implement all the details of registering callbacks and calling them. It has a dedicated vector of listeners and associated functions. This is OK, but essentially the same code and data will be repeated in potentially scores of places in the code.
- `UserInterface::OnKeyPress()` needs to be public so that the key pad can call it. I think there are some OO tricks we can use to get around this, but they add more boilerplate.
- The only reason the `IKeyListener` interface exists at all is to implement a Java-like listener API which depends on dynamic polymorphism. Thanks for nothing Gang-of-Four.

## Step 1. Dealing with the repeated code

Suppose we move the list of registered callbacks, and the function to invoke them, into their own class. We'd have a class like this:

```c++
class KeyListenerMgr
{
public:
    // Re-named from RegisterKeyListener().
    void Register(IKeyListener* listener)
    {
        m_listeners.push_back(listener);
    }

    // Re-named from OnKeyPress().
    void Call(KeyID key)
    {
        for (listener: m_listeners)
            listener->OnKeyPress(key);
    }

private:
    std::list<IKeyListener*> m_listeners;
};
```

With this class, we can refactor `KeyPad` by adding a member object:

```c++
class KeyPad
: public IKeyPad
{
public:
    // Exposed publicly for now. This exposes the Call() method, which is 
    // not really desirable. More later.
    KeyListenerMgr& KeyMgr() { return m_key_mgr; }

private:
    // This method could be refactored as a direct call since it has 
    // become a one-liner.
    void OnKeyPress(KeyID key)
    {
        m_key_mgr.Call(key);
    }

private:
    // This replaces the list we had before.
    KeyListenerMgr m_key_mgr;
};
```

The user interface is slightly modified to match:

```c++
class UserInterface
: public IUserInterface  // An interface for mocking purposes
, public IKeyListener    // An interface for event handling purposes
{
public:
    UserInterface(KeyPad& keypad)
    {
        keypad.KeyMgr().Register(this);
    }

    void OnKeyPress(KeyID key) override
    {
        // ...
    }
};
```

This simple change separates two areas of concern and allows the key pad to focus on being a key pad. It hasn't reduced the amount of code, but if we had two or more sources of key press events, we definitely would save some typing. But we can go further...

## Step 2. Dealing with the repeated code generically
 
The name of the method `OnKeyPress()` in `IKeyListener` and other similar interfaces is largely irrelevant, being buried inside `KeyListenerMgr`. Let's pretend that all of these listener interfaces will have only a single method and that it will be named `Call(...)`. Furthermore, for now, suppose that all these listener interfaces pass a single argument of type `Arg` to the callbacks. 

With these constraints in mind, we can easily make the listener manager generic:

```c++
// We can instantiate this template for any interface which has a method 
// of the form: `IAnyListener::Call(Arg arg) = 0;`
template <typename IAnyListener, typename Arg>
class ListenerMgr
{
public:
    void Register(IAnyListener* listener)
    {
        m_listeners.push_back(listener);
    }

    void Call(Arg arg)
    {
        for (auto& listener: m_listeners)
            listener->Call(arg);
    }

private:
    std::list<IAnyListener*> m_listeners;
};
```

With this simple template, we have captured in one place all the callback management data structures and loops in the application for all types of listener interface (with the assumptions mentioned above). We can rewrite `KeyPad` as follows.

```c++
class KeyPad
: public IKeyPad
{
public:
    using KeyListenerMgr = ListenerMgr<IKeyListener, KeyID>;
    KeyListenerMgr& KeyMgr() { return m_key_mgr; }

private:
    void OnKeyPress(KeyID key)
    {
        m_key_mgr.Call(key);
    }
private:
    KeyListenerMgr m_key_mgr;
};
```

We have to change the user interface a little to match:

```c++
class UserInterface
: public IUserInterface  // An interface for mocking purposes
, public IKeyListener    // An interface for event handling purposes
{
public:
    UserInterface(KeyPad& keypad)
    {
        keypad.KeyMgr().Register(this);
    }

    void Call(KeyID key) override
    {
        // ...
    }
};
```

The downside here is that the name of the callback method is too generic and meaningless. This is a temporary problem... 

## Step 3. Getting rid of the base class

The interface base class doesn't really add a lot of value, and it would be nice to lose it. To do this, we can factor out the dynamic polymorphism which underlies the listener framework. We want to assign a callback function and have no particular need of a base class to make it accessible to the caller. 

For the sake of argument, let's use `std::function` and pass a lambda for the callback. We would likely not use `std::function` for embedded projects, but it keeps the example simple. Now listener manager looks like this:

```c++
template <typename Arg>
// Re-named from ListenerMgr
class Callback
{
public:
    void Register(std::function<void(Arg)> listener)
    {
        m_listeners.push_back(listener);
    }

    void Call(Arg arg)
    {
        for (auto& listener: m_listeners)
            listener(arg);
    }

private:
    std::list<std::function<void(Arg)>> m_listeners;
};
```

The implementation of the user interface changes now to reflect this. It does not 
need to implement `IKeyListener`, which has become redundant:

```c++
class UserInterface
: public IUserInterface  // An interface for mocking purposes
{
public:
    UserInterface(KeyPad& keypad)
    {
        // Pass a lambda which invokes a member rather than a pointer the object.
        keypad.KeyMgr().Register([this](KeyID key){ HeyAKeyWasPressed(key); });
    }

private:
    // This method is private and non-virtual.
    void HeyAKeyWasPressed(KeyID key)
    {
        // ...
    }
};
```

At this point, we have not much changed the functionality. We still have a synchronous implementation of the Observer pattern. But: 
- We have eliminated the unnecesary dynamic polymorphism and the abstract base class. 
- We have eliminated a lot of duplicated or near duplicated code in event sources such as the key pad. 
- We have allowed callback functions to be private members.
- We have allowed callback functions to have arbitrary names which makes sense locally.

To my mind, this is a rather looser coupling between the `KeyPad` and `UserInterface`, and that's probably a good thing. 

# The downsides

I never really thought about this before, but one potential downside is that the Callback template represents a single callback function, but an abstract interface can have two or more functions in its API. I can see one or two use cases where having two Callback objects would be less efficient because there would be two lists. This has never come up as a concern.

# Different number of arguments

It is relatively simple to specialise the template to support zero, two or more callback arguments, using a variadic template. I have found that zero or one argument is sufficient since the one argument can be a structure. The implementation for embedded a bit simpler for the asynchronous case if we avoid variadic arguments.

# Adapting the template's interface

Although it has never been a problem in my experience, it is undesirable to expose too much of the `Callback` API to clients. Specifically, we would like to expose the `Register()` method but not the `Call()` method. This is easily done by using a little template adapter:

```c++
template <typename Arg>
class CallbackAdapter
{
public:
    CallbackAdapter(Callback<Arg>& callback)
    : m_callback{callback}
    {
    }

    void Register(std::function<void(Arg)> listener)
    {
        m_callback.Register(listener);
    }

private:
    Callback<Arg>& m_callback;
};
```

```c++
class KeyPad
: public IKeyPad
{
public:
    CallbackAdapter<KeyID> KeyMgr() { return {m_key_mgr}; }

private:
    void OnKeyPress(KeyID key)
    {
        m_key_mgr.Call(key);
    }
private:
    Callback<KeyID> m_key_mgr;
};
```

# Full code example

This section pulls everything together to show how to use the `Callback` type. The `Keypad` type now supports three callbacks, and two have the same signature.

```c++
class KeyPad
: public IKeyPad
{
public:
    CallbackAdapter<KeyID>   OnKeyPress()   { return m_on_key_press; }
    CallbackAdapter<KeyID>   OnKeyRelease() { return m_on_key_release; }
    CallbackAdapter<ErrorID> OnKeyError()   { return m_on_key_release; }

private:
    // These methods are implementation details of KeyPad, and are called from 
    // an ISR or elsewhere whenever a new key event is detected.
    void OnKeyPress(KeyID key)     { m_on_key_press.Call(key); }
    void OnKeyRelease(KeyID key)   { m_on_key_release.Call(key); }
    void OnKeyError(ErrorId error) { m_on_key_error.Call(error); }

    // Other details of the implementation....

private:
    // This represents a collection of callbacks with the signature void(KeyID).
    Callback<KeyID> m_on_key_press;
    Callback<KeyID> m_on_key_release;
    // This represents a collection of callbacks with the signature void(ErrorID).
    Callback<KeyID> m_on_key_error;
};
```

The `UserInterface` type is updated to listen to all three events:

```c++
class UserInterface
: public IUserInterface  // An interface for mocking purposes
{
public:
    UserInterface(KeyPad& keypad)
    {
        // As mentioned above, we have to register for each event individually. This has never been 
        // an issue. You could avoid a proliferation of Callback<> objects by, for example, passing 
        // a struct containing the type of event, e.g. struct KeyEvent { Type type; KeyID key;}.
        // We also only support a single callback argument (KeyID or ErrorID in this case). This has 
        // also never been an issue: use a struct.
        keypad.OnKeyPress().Register([this](KeyID key){ AKeyWasPressed(key); });
        keypad.OnKeyRelease().Register([this](KeyID key){ AKeyWasReleased(key); });
        keypad.OnKeyError().Register([this](ErrorID err ){ AKeyErrorHappened(err); });
    }

private:
    // This method is called when Keypad::OnKeyPress() is called.
    void AKeyWasPressed(KeyID key) 
    {
         // ...
    }
    // This method is called when Keypad::OnKeyRelease() is called.
    void AKeyWasReleased(KeyID key) 
    {
         // ...
    }
    // This method is called when Keypad::OnKeyError() is called.
    void AKeyErrorHappened(ErrorID err) 
    {
         // ...
    }
};
```

For the sake of argument, let's add a simple error logger which listens for events from multiple sources. Suppose we have a UART driver with an interface something like this:

```c++
enum class UARTErrorID {...};
class IUARTDriver
{
    virtual void Write(const uint8_t* buf, uint16_t len) = 0;
    virtual void Read(uint8_t* buf, uint16_t len) = 0;
    // The derived class will presumably have a member of type Callback<UARTErrorID> which it can return. 
    virtual CallbackAdapter<UARTErrorID> OnError() = 0;
}
```

And something similar for a SPI driver. Now the error logger might look like this:

```c++
class ErrorCatcher
{
public:
    ErrorCatcher(KeyPad& keypad, IUARTDriver& uart, ISPIDriver& spi)
    {
        // Register callbacks with several sources of errors.
        keypad.OnKeyError().Register([this](ErrorID err ){ KeypadError(err); });
        uart.OnError().Register([this](UARTErrorID err ){ UartError(err); });
        spi.OnError().Register([this](SPIErrorID err ){ SpiError(err); });
    }

private:
    // This method is called when Keypad::OnKeyError() is called, at the same 
    // time as UserInterface::AKeyWasPressed() is called (well, not literally parallel).
    void KeypadError(ErrorID err) 
    {
         // ...
    }
    void UartError(UartErrorID err) 
    {
         // ...
    }
    void SpiError(SpiErrorID err) 
    {
         // ...
    }
};
```

And here's a simple application:

```c++
int main()
{
    KeyPad     keypad;
    SPIDriver  spi1{...};  // Some configuration arguments.
    UARTDriver uart2{...}; // Some configuration arguments.

    UserInterface ui{keypad};
    ErrorCatcher error_log{keypad, uart2, spi1};

    ...
}
```

# Making event handling asynchronous

Nothing much has so far changed in the behaviour of the code: only its implementation. The next thing to consider is how to make the callbacks asynchronous. There is quite a lot of machinery involved in this so I won't go into details here. It basically boils down to having an event loop running which dispatches events as they appear in a queue. Rather than directly invoke the registered callbacks, we instead place an object into the event queue. Then, a little later that object is used to invoke the registered callbacks. This is all detailed elsewhere. 

**NOTE**: there is a class template called `Callback` in `otway_portable/Callback.h` which actually only supports a single registered callback. It could easily be 
modified to supported a list of registered callbacks but supporting only one is quite efficient for ISRs and the like.

**NOTE**: the behaviour descibed above is, more or less, implemented by a class template called `Signal` in `otway_portable/Signal.h`. This supports both synchronous and asynchronous callbacks, at the discretion of the event producer.

# Software timers

We typically use timeouts of various kinds all over the place, even for something as simple as toggling a heart beat LED. These are implemented by inheriting a base class and overriding the `Tick(uint32_t)` method in each class which needs to be ticked. Each such class needs to keep track of the current tick count and the tick count when it last did whatever it does periodically. This is quite a lot of boiler plate to include in dozens of classes. 

As an alternative, we can wrap up all the functionality in a `Timer` object. This internally deals with counting ticks, and uses a `Signal` object to manage connected callbacks, which it invokes when the timer fires. For example, a heart beat LED class would look like this (real code from the Otway demo):

```c++
class Blinky
{
public:
    // We pass in the digital output used to drive the LED and a toggle period.
    Blinky(eg::IDigitalOutput& led, uint32_t period_ms)
    : m_led{led}
    // Configure the member timer to emit an event every `period_ms`.
    , m_timer{period_ms, eg::Timer::Type::Repeating}
    {
        // Connect a private callback to the Signal object. This a more embedded-friendly 
        // equivalient of passing a std::function<void()> which holds a lambda.
        // Pass a pointer to member function as a template argument, and a pointer to 
        // the object on which to invoke it.
        m_timer.on_update().connect<&Blinky::on_timer>(this);
        m_timer.start();
    }

private:
    void on_timer()
    {
        m_led.toggle();
    }

private:
    eg::IDigitalOutput& m_led;
    eg::Timer           m_timer;
};
```

Note that the real implementation does not have a method called `Register()`, but `connect()`, which does the same thing. The code was inspired by Qt's Signals and Slots, in which Signals (event sources) are "connected" to Slots (callback methods). Also note that we are not passing lambdas wrapped in `std::function` objects as callbacks, as these require dynamic allocation. I did use lambdas and `std::function`, as well as `std::map` and `std::list`, in a Linux implementation of these ideas, and these avoided some quite fiddly shenanigans...

Using `Blinky` in an application is straightforward:

```c++
int main()
{
    // Function defined in the BSP.
    eg::IDigitalOutput& green_led();
    eg::IDigitalOutput& blue_led();

    // Create and start two blinking LEDs.
    Blinky green_blinky{green_led(), 100};
    Blinky blue_blinky{blue_led(), 150};

    // This runs the event loop which dispatches events emitted by Signals. 
    main_event_loop_run();
}
```
