from machine import Pin
import machine
import time

class BLHeliS:
    def __init__(self, pin_num):
        self.freq = 50
        pwm_pin = Pin(pin_num)
        self.pwm = machine.PWM(pwm_pin)
        self.pwm.freq(self.freq)

    # s varie de 0 à 1
    def set_speed(self, s):
        period = 1 / self.freq
        u16 = round(0.001 * (1 + s) * 65535 / period)
        self.pwm.duty_u16(u16)


if __name__ == "__main__":
    try:
        led = Pin(25, Pin.OUT)
        def blink(s):
            for i in range(s):
                led.toggle()
                time.sleep(0.5)
            led.off()
        print("5s delay")

        esc = BLHeliS(28)

        esc.set_speed(0)
        blink(5)
        
        target_speed = 0.95
        speed = 0.0
        a = 0.001
          
        while abs(speed - target_speed) > 0.01:
            speed += a * (target_speed - speed)
            print(speed)
            esc.set_speed(speed)
            time.sleep_ms(1)
        
        target_speed = 0.0

        while abs(speed - target_speed) > 0.01:
            speed += a * (target_speed - speed)
            print(speed)
            esc.set_speed(speed)
            time.sleep_ms(1)
        esc.set_speed(0)
        
        blink(10)

    except KeyboardInterrupt:
        esc.set_speed(0)