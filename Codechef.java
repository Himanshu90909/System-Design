// ================= DEVICE =================
/*
Abstraction:

- Device is abstract
- Device maintains ON/OFF state
- turnOn() and turnOff() are abstract operations
*/
abstract class Device {
    private boolean on;

    public abstract void turnOn();

    public abstract void turnOff();

    protected final boolean isOn() {
        return on;
    }

    protected final void setOn() {
        on = true;
    }

    protected final void setOff() {
        on = false;
    }
}


// ================= FAN =================
/*
Inheritance:

- Fan extends Device
- Prints "Fan ON" and "Fan OFF"
*/
class Fan extends Device {
    @Override
    public void turnOn() {
        if (isOn()) {
            return;
        }
        setOn();
        System.out.println("Fan ON");
    }

    @Override
    public void turnOff() {
        if (!isOn()) {
            return;
        }
        setOff();
        System.out.println("Fan OFF");
    }
}


// ================= LIGHT =================
/*
- Light extends Device
- Prints "Light ON" and "Light OFF"
*/
class Light extends Device {
    @Override
    public void turnOn() {
        if (isOn()) {
            return;
        }
        setOn();
        System.out.println("Light ON");
    }

    @Override
    public void turnOff() {
        if (!isOn()) {
            return;
        }
        setOff();
        System.out.println("Light OFF");
    }
}


// ================= CONTROLLER =================
class DeviceController {

    void operate(Device device) {
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
