import java.util.*;

// ================= BOOK =================
/*
Encapsulation:

- Stores book id, issued status, and the member currently holding the book.
- State is private and changed only through controlled methods.
*/
class Book {
    private final String id;
    private boolean issued;
    private Member issuedTo;

    public Book(String id) {
        this.id = id;
        this.issued = false;
        this.issuedTo = null;
    }

    public String getId() {
        return id;
    }

    public boolean isIssued() {
        return issued;
    }

    public Member getIssuedTo() {
        return issuedTo;
    }

    boolean issueTo(Member member) {
        if (issued) {
            return false;
        }
        issued = true;
        issuedTo = member;
        return true;
    }

    boolean releaseFrom(Member member) {
        if (!issued || issuedTo != member) {
            return false;
        }
        issued = false;
        issuedTo = null;
        return true;
    }
}

// ================= MEMBER =================
/*
Association + Dependency:

- A member has an id and can be associated with borrowed books.
- The issue/return methods temporarily use Book objects as parameters.
*/
class Member {
    private final String id;
    private final List<Book> borrowedBooks = new ArrayList<>();

    public Member(String id) {
        this.id = id;
    }

    public String getId() {
        return id;
    }

    public List<Book> getBorrowedBooks() {
        return Collections.unmodifiableList(borrowedBooks);
    }

    public void issueBook(Book book) {
        if (book.issueTo(this)) {
            borrowedBooks.add(book);
            System.out.println("Book " + book.getId() + " issued to " + id);
        }
    }

    public void returnBook(Book book) {
        if (book.releaseFrom(this)) {
            borrowedBooks.remove(book);
            System.out.println("Book " + book.getId() + " returned");
        }
    }
}

// ================= LIBRARY =================
/*
Aggregation:

- The library stores multiple books.
- Books are created independently and can exist without the library.
*/
class Library {
    private final List<Book> books = new ArrayList<>();

    public void addBook(Book book) {
        books.add(book);
    }

    public List<Book> getBooks() {
        return Collections.unmodifiableList(books);
    }
}

// ================= MAIN =================
public class Codechef {
    public static void main(String[] args) {

        Library lib = new Library();

        Book b1 = new Book("B1");
        Book b2 = new Book("B2");

        lib.addBook(b1);
        lib.addBook(b2);

        Member m = new Member("M1");

        m.issueBook(b1);
        m.returnBook(b1);
    }
}
