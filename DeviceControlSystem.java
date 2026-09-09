abstract class Device {
    private boolean on;

    public abstract void turnOn();

    public abstract void turnOff();

    protected final boolean isOn() {
        return on;
    }

    protected final void markOn() {
        on = true;
    }

    protected final void markOff() {
        on = false;
    }
}

class Fan extends Device {
    @Override
    public void turnOn() {
        if (isOn()) {
            return;
        }
        markOn();
        System.out.println("Fan turned ON");
    }

    @Override
    public void turnOff() {
        if (!isOn()) {
            return;
        }
        markOff();
        System.out.println("Fan turned OFF");
    }
}

class Light extends Device {
    @Override
    public void turnOn() {
        if (isOn()) {
            return;
        }
        markOn();
        System.out.println("Light turned ON");
    }

    @Override
    public void turnOff() {
        if (!isOn()) {
            return;
        }
        markOff();
        System.out.println("Light turned OFF");
    }
}

class DeviceController {
    public void operate(Device device) {
        device.turnOn();
        device.turnOff();
    }
}

public class DeviceControlSystem {
    public static void main(String[] args) {
        DeviceController controller = new DeviceController();

        Device fan = new Fan();
        Device light = new Light();

        controller.operate(fan);
        controller.operate(light);

        // Repeated operations are safely ignored when they are invalid.
        fan.turnOff();
        light.turnOn();
    }
}
