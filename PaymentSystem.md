# Payment System: Short Answer

## Abstraction

`PaymentMethod` is the abstraction. It is an interface that defines the common `pay()` operation for every payment type.

## Polymorphism

`Checkout.process()` accepts a `PaymentMethod` reference. At runtime, Java invokes the overridden `pay()` implementation of the actual object:

- `CardPayment` prints `Paid using Card`.
- `UPIPayment` prints `Paid using UPI`.

This is **runtime polymorphism**, also called **dynamic method dispatch**.

## Why not use `CardPayment` directly?

If `Checkout` directly depended on `CardPayment`, it would be tightly coupled to one payment type. Adding UPI or another payment method would require changing `Checkout`, reducing flexibility and testability and violating the **Open/Closed Principle**. Depending on the common `PaymentMethod` interface keeps `Checkout` loosely coupled and extensible.

## Run

```bash
javac PaymentSystem.java
java PaymentSystem
```

Expected output:

```text
Paid using Card
Paid using UPI
```
