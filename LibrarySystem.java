import java.util.ArrayList;
import java.util.List;

class Book {
    private final String title;
    private boolean borrowed;

    public Book(String title) {
        this.title = title;
    }

    public String getTitle() {
        return title;
    }

    public boolean isBorrowed() {
        return borrowed;
    }

    public void borrow() {
        if (borrowed) {
            throw new IllegalStateException("Book is already borrowed");
        }
        borrowed = true;
    }

    public void returnBook() {
        borrowed = false;
    }
}

class Library {
    // Aggregation: the library contains books, but does not own their lifecycle.
    private final List<Book> books = new ArrayList<>();

    public void addBook(Book book) {
        books.add(book);
    }

    public List<Book> getBooks() {
        return List.copyOf(books);
    }
}

class Member {
    private final String name;
    // Association: a member keeps references to books currently borrowed.
    private final List<Book> borrowedBooks = new ArrayList<>();

    public Member(String name) {
        this.name = name;
    }

    // Dependency: Book is used temporarily as a method parameter and through
    // its borrow() behavior; Member does not create or own the Book object.
    public void borrow(Book book) {
        book.borrow();
        borrowedBooks.add(book);
        System.out.println(name + " borrowed: " + book.getTitle());
    }

    public void returnBook(Book book) {
        book.returnBook();
        borrowedBooks.remove(book);
    }
}

public class LibrarySystem {
    public static void main(String[] args) {
        // Books can exist before and after the library that catalogs them.
        Book cleanCode = new Book("Clean Code");
        Book designPatterns = new Book("Design Patterns");

        Library library = new Library();
        library.addBook(cleanCode);
        library.addBook(designPatterns);

        Member member = new Member("Himanshu");
        member.borrow(cleanCode);
        member.returnBook(cleanCode);
    }
}
