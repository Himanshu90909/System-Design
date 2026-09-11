#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace restaurant {

enum class OrderType { DineIn, Delivery };
enum class OrderStatus { Pending, Accepted, Preparing, Ready, OutForDelivery, Delivered, Cancelled };

struct MenuItem {
    std::string id;
    std::string name;
    double price;
    bool available{true};
};

struct OrderLine {
    std::string itemId;
    int quantity;
};

struct Order {
    std::string id;
    std::string customerId;
    std::string restaurantId;
    std::vector<OrderLine> items;
    OrderType type;
    std::optional<std::string> deliveryAddress;
    double itemsTotal{0.0};
    double deliveryFee{0.0};
    OrderStatus status{OrderStatus::Pending};
    std::optional<std::string> agentId;
};

class RestaurantManagementSystem {
    std::unordered_map<std::string, std::vector<MenuItem>> menus_;
    std::unordered_map<std::string, Order> orders_;
    std::unordered_map<std::string, std::vector<std::string>> agentOrders_;
    std::size_t nextOrderId_{1};

    static void require(bool condition, const std::string& message) {
        if (!condition) throw std::invalid_argument(message);
    }

public:
    void addMenuItem(const std::string& restaurantId, MenuItem item) {
        require(!restaurantId.empty() && !item.id.empty() && item.price >= 0, "invalid menu item");
        auto& menu = menus_[restaurantId];
        auto existing = std::find_if(menu.begin(), menu.end(), [&](const MenuItem& current) { return current.id == item.id; });
        if (existing != menu.end()) *existing = std::move(item);
        else menu.push_back(std::move(item));
    }

    std::string placeOrder(const std::string& customerId, const std::string& restaurantId,
                           std::vector<OrderLine> items, OrderType type,
                           std::optional<std::string> deliveryAddress = std::nullopt) {
        require(!customerId.empty() && !restaurantId.empty() && !items.empty(), "customer, restaurant, and items are required");
        require(type == OrderType::DineIn || deliveryAddress.has_value(), "delivery address is required");

        const auto menuIt = menus_.find(restaurantId);
        require(menuIt != menus_.end(), "restaurant menu not found");
        double subtotal = 0;
        for (const auto& line : items) {
            require(line.quantity > 0, "quantity must be positive");
            auto item = std::find_if(menuIt->second.begin(), menuIt->second.end(), [&](const MenuItem& candidate) { return candidate.id == line.itemId; });
            require(item != menuIt->second.end() && item->available, "menu item is unavailable");
            subtotal += item->price * line.quantity;
        }

        const std::string id = "order-" + std::to_string(nextOrderId_++);
        const double deliveryFee = type == OrderType::Delivery ? 4.99 : 0.0;
        orders_.emplace(id, Order{id, customerId, restaurantId, std::move(items), type, std::move(deliveryAddress), subtotal, deliveryFee});
        return id;
    }

    void acceptOrder(const std::string& orderId) { transition(orderId, OrderStatus::Accepted, OrderStatus::Pending); }
    void startPreparing(const std::string& orderId) { transition(orderId, OrderStatus::Preparing, OrderStatus::Accepted); }
    void markReady(const std::string& orderId) { transition(orderId, OrderStatus::Ready, OrderStatus::Preparing); }

    void assignDeliveryAgent(const std::string& orderId, const std::string& agentId) {
        auto& order = findOrder(orderId);
        require(order.type == OrderType::Delivery && order.status == OrderStatus::Ready, "order is not ready for delivery");
        order.agentId = agentId;
        order.status = OrderStatus::OutForDelivery;
        agentOrders_[agentId].push_back(orderId);
    }

    void markDelivered(const std::string& orderId) {
        auto& order = findOrder(orderId);
        require(order.status == OrderStatus::OutForDelivery || order.status == OrderStatus::Ready, "order cannot be delivered yet");
        order.status = OrderStatus::Delivered;
    }

    const Order& getOrder(const std::string& orderId) const {
        auto it = orders_.find(orderId);
        if (it == orders_.end()) throw std::out_of_range("order not found");
        return it->second;
    }

    double total(const std::string& orderId) const {
        const auto& order = getOrder(orderId);
        return order.itemsTotal + order.deliveryFee;
    }

private:
    Order& findOrder(const std::string& orderId) {
        auto it = orders_.find(orderId);
        if (it == orders_.end()) throw std::out_of_range("order not found");
        return it->second;
    }

    void transition(const std::string& orderId, OrderStatus next, OrderStatus expected) {
        auto& order = findOrder(orderId);
        require(order.status == expected, "invalid order state transition");
        order.status = next;
    }
};

} // namespace restaurant

int main() {
    using namespace restaurant;
    RestaurantManagementSystem system;
    system.addMenuItem("r-italian", {"pizza", "Margherita Pizza", 12.50});
    system.addMenuItem("r-italian", {"pasta", "Pasta Primavera", 10.00});

    const auto orderId = system.placeOrder("customer-1", "r-italian", {{"pizza", 2}, {"pasta", 1}},
                                          OrderType::Delivery, "123 Main Street");
    system.acceptOrder(orderId);
    system.startPreparing(orderId);
    system.markReady(orderId);
    system.assignDeliveryAgent(orderId, "agent-7");
    system.markDelivered(orderId);

    const auto& order = system.getOrder(orderId);
    if (order.status != OrderStatus::Delivered || system.total(orderId) != 39.99) return 1;
    std::cout << "Restaurant order checks passed\n";
    std::cout << orderId << " total=$" << std::fixed << std::setprecision(2) << system.total(orderId) << "\n";
}
