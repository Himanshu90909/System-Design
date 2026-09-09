# Device Control System

This implementation demonstrates **abstraction**, **inheritance**, **polymorphism**, and safe state handling.

## Design

- `ManagedDevice` is an abstract base class that declares `turnOn()` and `turnOff()`. It privately stores the ON/OFF state.
- `ManagedFan` and `ManagedLight` inherit from `ManagedDevice` and provide their own implementations of both operations.
- `ManagedDeviceController.operate(ManagedDevice device)` works with the base type and calls `turnOn()` and `turnOff()` without knowing the concrete device type. Java dispatches each call to the correct subclass implementation at runtime.
- Each subclass checks the current state before changing it. Turning on an already-on device or turning off an already-off device has no effect.
- A new device can extend `ManagedDevice` and implement the two methods without changing `ManagedDeviceController`. No `instanceof` or type checking is used.

## Expected behavior

Calling `operate()` on a fan produces:

```text
Fan turned ON
Fan turned OFF
```

Calling it on a light produces:

```text
Light turned ON
Light turned OFF
```

## Run

```bash
javac DeviceControlSystem.java
java DeviceControlSystem
```
