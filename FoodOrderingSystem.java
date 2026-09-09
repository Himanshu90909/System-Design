import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

final class User {
    private final int id;
    private final List<Order> orders = new ArrayList<>();

    public User(int id) {
        this.id = id;
    }

    public int getId() {
        return id;
    }

    public List<Order> getOrders() {
        return Collections.unmodifiableList(orders);
    }

    // Maintains the User -> Order association without exposing the collection.
    void addOrder(Order order) {
        orders.add(order);
    }
}

final class FoodItem {
    private final String name;
    private final double price;
    private final int quantity;

    public FoodItem(String name, double price, int quantity) {
        if (name == null || name.isBlank()) {
            throw new IllegalArgumentException("Food item name is required");
        }
        if (price < 0 || quantity <= 0) {
            throw new IllegalArgumentException("Price must be non-negative and quantity must be positive");
        }
        this.name = name;
        this.price = price;
        this.quantity = quantity;
    }

    public String getName() {
        return name;
    }

    public double getPrice() {
        return price;
    }

    public int getQuantity() {
        return quantity;
    }
}

final class Order {
    private final int orderId;
    private final User user;
    private final List<FoodItem> foodItems;

    Order(int orderId, User user, List<FoodItem> foodItems) {
        if (user == null || foodItems == null || foodItems.isEmpty()) {
            throw new IllegalArgumentException("An order requires a user and at least one food item");
        }
        this.orderId = orderId;
        this.user = user;
        // Aggregation: the order stores existing FoodItem objects without owning
        // their independent lifecycle.
        this.foodItems = new ArrayList<>(foodItems);
    }

    public int getOrderId() {
        return orderId;
    }

    public User getUser() {
        return user;
    }

    public List<FoodItem> getFoodItems() {
        return Collections.unmodifiableList(foodItems);
    }

    public double getTotalCost() {
        double total = 0.0;
        for (FoodItem item : foodItems) {
            total += item.getPrice() * item.getQuantity();
        }
        return total;
    }
}

final class OrderService {
    public Order createOrder(User user, int orderId, List<FoodItem> foodItems) {
        Order order = new Order(orderId, user, foodItems);
        user.addOrder(order);
        return order;
    }
}

public class FoodOrderingSystem {
    public static void main(String[] args) {
        User user = new User(101);
        List<FoodItem> items = List.of(
                new FoodItem("Veg Burger", 120.0, 2),
                new FoodItem("French Fries", 80.0, 1),
                new FoodItem("Cold Drink", 50.0, 2)
        );

        OrderService orderService = new OrderService();
        Order order = orderService.createOrder(user, 5001, items);

        System.out.println(order.getTotalCost());
    }
}
