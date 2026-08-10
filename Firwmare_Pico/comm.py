from machine import UART, Pin, ADC
import uasyncio as asyncio
from blheli_s import BLHeliS as ESC

uart = UART(0, baudrate = 921600, tx = Pin(0), rx = Pin(1))
uart.write("Hello, World!\n")

bat = ADC(Pin(27))

esc = ESC(28)

async def uart_task():
    reader = asyncio.StreamReader(uart)
    while True:
        line = await reader.readline()
        print(f"received: {line}")
        print(line[0])
        if line[0] != ord('#'):
            continue
        line = line[1:]
        line = line.decode('utf8').strip().split(' ')
        print(f"line = {line}")
        cmd = line[0]
        if len(line) > 1:
            args = line[1:]
        else:
            args = []
        print(f"cmd={cmd}, args={args}")
        try:
            if cmd == 'BAT':
                uart.write(f"BAT {bat.read_u16()}\n")
            elif cmd == 'MOT':
                esc.set_speed(float(args[0]))
            else:
                print("invalid command")
        except IndexError:
            print("invalid arguments")

asyncio.run(uart_task())