# Library System: Short Answer

## Relationship types

- **`Library → Book`: Aggregation**. A library contains or catalogs books, but a `Book` can exist independently of the `Library`. The library does not own the book's lifecycle.
- **`Member → Book`: Association**. A member is linked to a book while borrowing it. The relationship can be temporary and may change when the book is returned.

## Dependency in the borrow operation

Dependency is represented by the `Member.borrow(Book book)` method. `Member` temporarily uses a `Book` object passed as a parameter and calls `book.borrow()` and `book.getTitle()`. `Member` does not create or permanently own that `Book` object.

## Lifecycle and interaction properties

| Relationship | Property |
|---|---|
| **Aggregation** | Weak whole-part relationship: the `Book` can exist independently and can outlive the `Library`. |
| **Association** | Structural link between independent objects: a `Member` may be associated with a borrowed `Book` temporarily. |
| **Dependency** | Short-term usage relationship: `Member` depends on the `Book` parameter while executing `borrow()`, but does not retain ownership of its lifecycle. |

## Design summary

- `Library` stores a collection of existing `Book` objects.
- `Member` stores references to books currently borrowed.
- `Book` controls its own borrowed/available state.
- `Member.borrow(Book)` demonstrates dependency through method-parameter usage.

## Run

```bash
javac LibrarySystem.java
java LibrarySystem
```

Expected output:

```text
Himanshu borrowed: Clean Code
```
