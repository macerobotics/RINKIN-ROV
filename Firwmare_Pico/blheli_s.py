from machine import Pin, ADC
import time

try:
    freq = 50
    
    led = machine.Pin(25, Pin.OUT)

    pwm_pin = machine.Pin(28)
    pwm = machine.PWM(pwm_pin)

    pwm.freq(freq)

    # s varie de 0 à 1
    def set_speed(s):
        period = 1 / freq
        u16 = round(0.001 * (1 + s) * 65535 / period)
        #print(f"u16 = {u16}, period = {period}")
        #print(f"{u16 / 65535 * period * 1000} ms")
        pwm.duty_u16(u16)
        
    def blink(s):
        for i in range(s):
            led.toggle()
            time.sleep(0.5)
        led.off()
    
    print("5s delay")
    set_speed(0)
    blink(5)
    

    target_speed = 0.95
    speed = 0.0
    a = 0.001
    
    
    while abs(speed - target_speed) > 0.01:
        speed += a * (target_speed - speed)
        print(speed)
        set_speed(speed)
        time.sleep_ms(1)
    
    target_speed = 0.0

    while abs(speed - target_speed) > 0.01:
        speed += a * (target_speed - speed)
        print(speed)
        set_speed(speed)
        time.sleep_ms(1)
    set_speed(0)
    
    blink(10)
        
except KeyboardInterrupt:
    pwm.deinit()
