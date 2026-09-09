// ================= DEVICE =================
abstract class Device {
    private boolean on = false;

    public abstract void turnOn();

    public abstract void turnOff();

    protected boolean isOn() {
        return on;
    }

    protected void setOn(boolean value) {
        on = value;
    }
}

// ================= FAN =================
class Fan extends Device {
    @Override
    public void turnOn() {
        if (!isOn()) {
            setOn(true);
            System.out.println("Fan ON");
        }
    }

    @Override
    public void turnOff() {
        if (isOn()) {
            setOn(false);
            System.out.println("Fan OFF");
        }
    }
}

// ================= LIGHT =================
class Light extends Device {
    @Override
    public void turnOn() {
        if (!isOn()) {
            setOn(true);
            System.out.println("Light ON");
        }
    }

    @Override
    public void turnOff() {
        if (isOn()) {
            setOn(false);
            System.out.println("Light OFF");
        }
    }
}

// ================= CONTROLLER =================
class DeviceController {
    public void operate(Device device) {
        device.turnOn();
        device.turnOff();
    }
}

// ================= MAIN =================
public class Codechef {
    public static void main(String[] args) {
        DeviceController controller = new DeviceController();

        Device fan = new Fan();
        Device light = new Light();

        controller.operate(fan);
        controller.operate(light);
    }
}
