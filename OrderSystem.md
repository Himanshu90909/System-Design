# Online Order System: Short Answer

## Design issue

A single class that validates users, creates orders, processes payments, and sends confirmation messages has **multiple reasons to change**. This violates the **Single Responsibility Principle (SRP)** and creates a **God class** with high coupling and low cohesion.

For example, a change to payment-provider logic, user-validation rules, or email/SMS delivery would all require changing the same class. That makes the class harder to test, maintain, and extend. It also tends to violate the **Open/Closed Principle**, because supporting a new payment provider or notification channel requires modifying the orchestration class instead of adding a new implementation.

The better design is to let `OrderService` coordinate the order workflow while delegating each specialized responsibility to a focused service.

## Relationship types

| Relationship | Type | Explanation |
|---|---|---|
| `OrderService → UserService` | **Association**, specifically a **dependency / “uses-a” relationship** | `OrderService` uses `UserService` to validate a user while placing an order. It does not own the user service. |
| `OrderService → PaymentService` | **Association**, specifically a **dependency / “uses-a” relationship** | `OrderService` delegates payment processing to `PaymentService`. The payment service can be replaced without changing the order workflow. |
| `OrderService → NotificationService` | **Association**, specifically a **dependency / “uses-a” relationship** | `OrderService` uses the notification service to send a confirmation after successful payment. It does not own the notification service. |

In the code, these dependencies are injected through the `OrderService` constructor and referenced through interfaces. This is **dependency inversion** and supports loose coupling, polymorphism, and unit testing with mocks or stubs.

These are **not** inheritance (`is-a`), composition, or aggregation relationships: `OrderService` does not represent a kind of user, payment service, or notification service, and it does not manage their lifetimes.

## Code

See [`OrderSystem.cpp`](OrderSystem.cpp) for a complete compilable C++ example.

The design separates responsibilities as follows:

- `UserService`: user validation.
- `PaymentService`: payment processing.
- `NotificationService`: confirmation delivery.
- `OrderService`: orchestration of the order workflow.

The `OrderService` receives interface references, so the concrete implementations can be changed independently—for example, replacing card payments with a wallet provider or email notifications with SMS notifications.

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -pedantic OrderSystem.cpp -o OrderSystem
./OrderSystem
```

Expected output:

```text
Processing payment of $249.99.
Confirmation sent for order #1 to user 101.
```
