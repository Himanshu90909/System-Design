#include <iostream>
#include <memory>
#include <string>
#include <utility>

// A small domain object containing only order data.
struct Order {
    int id;
    int userId;
    double amount;
};

// Each interface represents one responsibility.
class UserService {
public:
    virtual ~UserService() = default;
    virtual bool isValidUser(int userId) const = 0;
};

class PaymentService {
public:
    virtual ~PaymentService() = default;
    virtual bool processPayment(double amount) = 0;
};

class NotificationService {
public:
    virtual ~NotificationService() = default;
    virtual void sendOrderConfirmation(const Order& order) const = 0;
};

// OrderService coordinates the use cases; it does not implement the details
// of validation, payment, or message delivery.
class OrderService {
private:
    // Non-owning references express "uses-a" dependencies. The collaborators
    // must outlive OrderService.
    UserService& userService;
    PaymentService& paymentService;
    NotificationService& notificationService;
    int nextOrderId = 1;

public:
    OrderService(UserService& userService,
                 PaymentService& paymentService,
                 NotificationService& notificationService)
        : userService(userService),
          paymentService(paymentService),
          notificationService(notificationService) {}

    bool placeOrder(int userId, double amount) {
        if (!userService.isValidUser(userId)) {
            std::cout << "Order rejected: invalid user.\n";
            return false;
        }

        Order order{nextOrderId++, userId, amount};

        if (!paymentService.processPayment(order.amount)) {
            std::cout << "Order rejected: payment failed.\n";
            return false;
        }

        notificationService.sendOrderConfirmation(order);
        return true;
    }
};

// Simple implementations used by the example.
class BasicUserService final : public UserService {
public:
    bool isValidUser(int userId) const override {
        return userId > 0;
    }
};

class CardPaymentService final : public PaymentService {
public:
    bool processPayment(double amount) override {
        std::cout << "Processing payment of $" << amount << ".\n";
        return amount > 0;
    }
};

class EmailNotificationService final : public NotificationService {
public:
    void sendOrderConfirmation(const Order& order) const override {
        std::cout << "Confirmation sent for order #" << order.id
                  << " to user " << order.userId << ".\n";
    }
};

int main() {
    BasicUserService userService;
    CardPaymentService paymentService;
    EmailNotificationService notificationService;

    OrderService orderService(userService, paymentService, notificationService);
    orderService.placeOrder(101, 249.99);

    return 0;
}
