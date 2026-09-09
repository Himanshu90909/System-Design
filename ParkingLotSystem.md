# Parking Lot System: Short Answer

## Relationship types

- **`ParkingLot → ParkingSpot`: Aggregation**. The parking lot contains and manages parking spots, but a parking spot can exist independently of a particular parking lot.
- **`Vehicle → ParkingSpot`: Association**. A vehicle is assigned to a parking spot temporarily. The vehicle and spot can exist independently, and the assignment changes when the vehicle exits.

## Pricing logic

A dedicated **`ParkingFeeCalculator`** should handle pricing logic because separating pricing from parking-space management improves **separation of concerns and maintainability**. Pricing rules can change without modifying `ParkingLot`, making both classes easier to understand, test, and maintain.

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
