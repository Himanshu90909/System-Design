# Codechef Library System Solution

This solution completes the provided template using encapsulation, association, dependency, and aggregation.

- `Book` keeps its ID, issued state, and current member private. Controlled methods update the issue state.
- `Member` is associated with borrowed books and depends on `Book` objects passed to `issueBook()` and `returnBook()`.
- `Library` aggregates a collection of books. Books are created independently and can exist without the library.
- A book cannot be issued while it is already issued.
- Issuing and returning update both sides of the relationship: the book records its member, and the member records its borrowed book.

Expected output:

```text
Book B1 issued to M1
Book B1 returned
```

Run it with:

```bash
javac Codechef.java
java Codechef
```
