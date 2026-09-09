# Parking Lot System: Short Answer

## Relationship types

- **`ParkingLot → ParkingSpot`: Composition**. The parking lot creates and manages its parking spots. The spots are parts of that lot and their lifecycle is controlled by it.
- **`Vehicle → ParkingSpot`: Association**. A vehicle is assigned to a parking spot temporarily. The vehicle and spot can exist independently, and the assignment changes when the vehicle exits.

## Pricing logic

A dedicated **`ParkingFeeCalculator`** should handle pricing logic because fee calculation is a separate responsibility from parking-space management. This follows the **Single Responsibility Principle** and allows pricing rules to change without modifying `ParkingLot`.

## Code design

- `ParkingLot` manages available spots and parking/exit operations.
- `ParkingSpot` tracks whether it is occupied.
- `Vehicle` stores vehicle information.
- `ParkingFeeCalculator` calculates the fee using entry and exit times.

## Run

```bash
javac ParkingLotSystem.java
java ParkingLotSystem
```

Expected output:

```text
Vehicle parked in spot: 1
Parking fee: Rs. 150.0
```
