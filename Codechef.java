import java.util.*;

// ================= USER =================
/*
Encapsulation:

- id is private
- user can have multiple orders (association)
*/
class User {
    private final String id;
    private final List<Order> orders;

    public User(String id) {
        this.id = id;
        this.orders = new ArrayList<>();
    }

    public String getId() {
        return id;
    }

    public List<Order> getOrders() {
        return Collections.unmodifiableList(orders);
    }

    public void addOrder(Order order) {
        orders.add(order);
    }
}

// ================= FOOD ITEM =================
/*
Encapsulation:

- name, price, quantity are private
*/
class FoodItem {
    private final String name;
    private final double price;
    private final int quantity;

    public FoodItem(String name, double price, int quantity) {
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

// ================= ORDER =================
/*
Association:

- Order HAS-A User

Aggregation:

- Order HAS-A List<FoodItem>

Also:

- implement total cost calculation
*/
class Order {
    private final String id;
    private final User user;
    private final List<FoodItem> items;

    public Order(String id, User user, List<FoodItem> items) {
        this.id = id;
        this.user = user;
        this.items = new ArrayList<>(items);
    }

    public String getId() {
        return id;
    }

    public User getUser() {
        return user;
    }

    public List<FoodItem> getItems() {
        return Collections.unmodifiableList(items);
    }

    public double getTotalCost() {
        double total = 0.0;
        for (FoodItem item : items) {
            total += item.getPrice() * item.getQuantity();
        }
        return total;
    }
}

// ================= SERVICE =================
class OrderService {
    private int counter = 1;

    /*
    Create order:
    - generate id ORD1, ORD2...
    - assign to user
    */
    public Order createOrder(User user, List<FoodItem> items) {
        String orderId = "ORD" + counter++;
        Order order = new Order(orderId, user, items);
        user.addOrder(order);
        return order;
    }
}

// ================= MAIN =================
public class Codechef {
    public static void main(String[] args) {

        User user = new User("U1");

        List<FoodItem> items = new ArrayList<>();
        items.add(new FoodItem("Pizza", 100, 2));
        items.add(new FoodItem("Burger", 50, 1));

        OrderService service = new OrderService();
        Order order = service.createOrder(user, items);

        System.out.println(order.getTotalCost());
    }
}
