abstract class ManagedDevice {
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

class ManagedFan extends ManagedDevice {
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

class ManagedLight extends ManagedDevice {
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

class ManagedDeviceController {
    public void operate(ManagedDevice device) {
        device.turnOn();
        device.turnOff();
    }
}

public class DeviceControlSystem {
    public static void main(String[] args) {
        ManagedDeviceController controller = new ManagedDeviceController();

        ManagedDevice fan = new ManagedFan();
        ManagedDevice light = new ManagedLight();

        controller.operate(fan);
        controller.operate(light);

        // Repeated operations are safely ignored when they are invalid.
        fan.turnOff();
        light.turnOn();
    }
}
