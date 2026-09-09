import java.time.Duration;
import java.time.Instant;
import java.util.ArrayList;
import java.util.List;

class Vehicle {
    private final String registrationNumber;

    public Vehicle(String registrationNumber) {
        this.registrationNumber = registrationNumber;
    }

    public String getRegistrationNumber() {
        return registrationNumber;
    }
}

class ParkingSpot {
    private final int number;
    private Vehicle vehicle;

    public ParkingSpot(int number) {
        this.number = number;
    }

    public boolean isAvailable() {
        return vehicle == null;
    }

    public void park(Vehicle vehicle) {
        if (!isAvailable()) {
            throw new IllegalStateException("Parking spot is occupied");
        }
        this.vehicle = vehicle;
    }

    public Vehicle removeVehicle() {
        if (vehicle == null) {
            throw new IllegalStateException("Parking spot is already empty");
        }
        Vehicle parkedVehicle = vehicle;
        vehicle = null;
        return parkedVehicle;
    }

    public int getNumber() {
        return number;
    }
}

class ParkingFeeCalculator {
    private final double hourlyRate;

    public ParkingFeeCalculator(double hourlyRate) {
        this.hourlyRate = hourlyRate;
    }

    public double calculateFee(Instant entryTime, Instant exitTime) {
        long minutes = Math.max(1, Duration.between(entryTime, exitTime).toMinutes());
        long billableHours = (minutes + 59) / 60;
        return billableHours * hourlyRate;
    }
}

class ParkingLot {
    // Aggregation: the lot manages a collection of parking spots, while a
    // parking spot can conceptually exist independently of the lot.
    private final List<ParkingSpot> spots = new ArrayList<>();
    private final ParkingFeeCalculator feeCalculator;

    public ParkingLot(int numberOfSpots, ParkingFeeCalculator feeCalculator) {
        this.feeCalculator = feeCalculator;
        for (int number = 1; number <= numberOfSpots; number++) {
            spots.add(new ParkingSpot(number));
        }
    }

    public ParkingSpot park(Vehicle vehicle) {
        ParkingSpot availableSpot = spots.stream()
                .filter(ParkingSpot::isAvailable)
                .findFirst()
                .orElseThrow(() -> new IllegalStateException("Parking lot is full"));

        // Association: this vehicle is assigned to a spot temporarily.
        availableSpot.park(vehicle);
        return availableSpot;
    }

    public double exit(ParkingSpot spot, Instant entryTime, Instant exitTime) {
        spot.removeVehicle();
        return feeCalculator.calculateFee(entryTime, exitTime);
    }
}

public class ParkingLotSystem {
    public static void main(String[] args) {
        ParkingFeeCalculator feeCalculator = new ParkingFeeCalculator(50.0);
        ParkingLot parkingLot = new ParkingLot(2, feeCalculator);
        Vehicle vehicle = new Vehicle("MH-12-AB-1234");

        Instant entryTime = Instant.parse("2026-09-09T10:00:00Z");
        Instant exitTime = Instant.parse("2026-09-09T12:30:00Z");

        ParkingSpot assignedSpot = parkingLot.park(vehicle);
        double fee = parkingLot.exit(assignedSpot, entryTime, exitTime);

        System.out.println("Vehicle parked in spot: " + assignedSpot.getNumber());
        System.out.println("Parking fee: Rs. " + fee);
    }
}
