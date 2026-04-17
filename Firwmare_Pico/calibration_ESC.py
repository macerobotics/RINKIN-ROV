# programme pour la calibration des ESC
# A faire une fois par ESC
# Version : 0.1
# Date de modification : 13/04/26 

import utime
from machine import Pin,PWM,UART

fireup_pin = machine.Pin(18,Pin.IN,Pin.PULL_UP)
led_bultin = machine.Pin(25,machine.Pin.OUT)
led1 = machine.Pin(7,machine.Pin.OUT)
led2 = machine.Pin(8,machine.Pin.OUT)
raspi = machine.UART(0, baudrate=9600 , tx=Pin(16), rx=Pin(17)) # Nico
esc = []

for n in range(5):
    esc.append(PWM(Pin(n, mode=Pin.OUT),freq=50))
moters,leds,trig,count = [0] * 5,[0] * 2,0,0

print("CALIBRATION ESC...")

def calibration_ESC():
    for n in range(5):
        esc[n].duty_u16(6350)
    print("throttle in top position")
    for n in range(5,1,-1):
        print(n)
        utime.sleep(2)
    print("throttle in neutral position")
    for n in range(5):
        esc[n].duty_u16(4950)
    for n in range(7,1,-1):
        print(n)
        utime.sleep(3)
    print("ESC fired up")
    for n in range(3):
        led_bultin.value(1)
        utime.sleep(0.1)
        led_bultin.value(0)
        utime.sleep(0.1)

##################################################################
# Calibration des ESC
calibration_ESC()

# FIN 
