# Codechef Online Food Ordering Solution

This solution completes the provided template without changing its required class and method structure.

- `User` uses **encapsulation** with private `id` and `orders` fields. It maintains the `User → Order` association through `addOrder()`.
- `Order` stores a `User` reference, representing the order's association with its user.
- `Order` stores a copied list of `FoodItem` objects, representing **aggregation** because food items can exist independently of an order.
- `Order.getTotalCost()` dynamically computes the sum of `price × quantity` for every item.
- `OrderService.createOrder()` generates IDs such as `ORD1`, assigns the order to the user, and returns the created order.

For the sample input, the output is:

```text
250.0
```

Run it with:

```bash
javac Codechef.java
java Codechef
```
