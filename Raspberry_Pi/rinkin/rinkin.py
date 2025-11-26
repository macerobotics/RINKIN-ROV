#!/usr/bin/python3
import asyncio
import socket
import serial
from smbus2 import SMBus
from cmps12 import CMPS12

udp_port = 1234
addr = None

async def imu_loop(sock):
    imu = CMPS12(SMBus(1))
    loop = asyncio.get_running_loop()
    while True:
        if addr is not None:
            imu.update()
            print(f"heading = {imu.heading()}\tpich={imu.pitch()}\troll={imu.roll()}")
            await loop.sock_sendto(sock, f"#heading,{imu.heading()}!".encode("utf-8"), addr)
            await loop.sock_sendto(sock, f"#pitch,{imu.pitch()}!".encode("utf-8"), addr)
            await loop.sock_sendto(sock, f"#roll,{imu.roll()}!".encode("utf-8"), addr)
        await asyncio.sleep(0.1)

async def udp_loop(sock, ser):
    global addr
    loop = asyncio.get_running_loop()
    while True:
        data, addr = await loop.sock_recvfrom(sock, 1500)
        print(f"received: {data} from {addr}")
        loop.call_soon(ser.write, data)

"""
def on_message(client, userdata, msg):
    print(f"topic: {msg.topic}, payload: {msg.payload.decode()}")
    if msg.topic == "/motor":
        speed = float(msg.payload.rstrip(b'\x00'))
        speed = max(0, min(speed, 0.95))
        cmd = f"#MOT {speed}\n".encode('utf8')
        print(f"> {repr(cmd)}")
        ser.write(cmd)
"""

async def main():
    ser = serial.Serial("/dev/ttyS0", 921600)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(("0.0.0.0", udp_port))
    sock.setblocking(False)
    imu_task = asyncio.create_task(imu_loop(sock))
    udp_task = asyncio.create_task(udp_loop(sock, ser))
    await imu_task
    await udp_task
    ser.close()

asyncio.run(main())




    




"""

def motor(address, *args):
    print(f"{address:}: {args}")
    ser.write(int.to_bytes(args[0]))

ser = serial.Serial("/dev/ttyS0", 115200)

ser.close()

"""