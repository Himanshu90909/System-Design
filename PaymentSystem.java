interface PaymentMethod {
    void pay();
}

class CardPayment implements PaymentMethod {
    @Override
    public void pay() {
        System.out.println("Paid using Card");
    }
}

class UPIPayment implements PaymentMethod {
    @Override
    public void pay() {
        System.out.println("Paid using UPI");
    }
}

class Checkout {
    public void process(PaymentMethod method) {
        method.pay();
    }
}

public class PaymentSystem {
    public static void main(String[] args) {
        Checkout checkout = new Checkout();

        checkout.process(new CardPayment());
        checkout.process(new UPIPayment());
    }
}
