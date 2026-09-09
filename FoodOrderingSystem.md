# Online Food Ordering System

This implementation demonstrates the requested low-level design using **encapsulation**, **association**, and **aggregation**.

## Design

`User` has a private identifier and a private collection of orders. The `User → Order` relationship is an **association**: one user can be linked to many orders, and each order stores a reference back to its user.

`Order` has a private order ID, a reference to its user, and a private collection of food items. The `Order → FoodItem` relationship is an **aggregation**: an order stores existing food items, while food items remain independently creatable and are not lifecycle-owned by the order.

All fields are private. Constructors establish valid state, getters expose read-only views or values, and callers cannot directly replace the internal collections. The `OrderService` creates the order and registers it with the user, keeping order-creation coordination outside the entity classes.

## Total cost

`Order.getTotalCost()` calculates the value dynamically from the actual items:

```text
sum(foodItem.price × foodItem.quantity)
```

For the sample data, the result is `450.0`.

## Run

```bash
javac FoodOrderingSystem.java
java FoodOrderingSystem
```

Expected output:

```text
450.0
```
