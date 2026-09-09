# Codechef Device Control Solution

This solution completes the provided template using abstraction, inheritance, and polymorphism.

- `Device` is an **abstract class** with private ON/OFF state and abstract `turnOn()` and `turnOff()` methods.
- `Fan` and `Light` **inherit** from `Device` and provide their own implementations.
- `DeviceController.operate(Device device)` accepts the common `Device` type and invokes both methods polymorphically.
- Repeating an invalid operation is safe: turning on an already-on device or turning off an already-off device does not change state or print another message.
- No `instanceof` or device-specific type checking is used.

Expected output:

```text
Fan ON
Fan OFF
Light ON
Light OFF
```

Run it with:

```bash
javac Codechef.java
java Codechef
```
