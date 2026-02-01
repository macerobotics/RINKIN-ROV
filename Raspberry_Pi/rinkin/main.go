package main

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"os"
	"time"

	"go.bug.st/serial"
	"periph.io/x/conn/v3/i2c"
	"periph.io/x/conn/v3/i2c/i2creg"
	"periph.io/x/host/v3"
)

func main() {
	addr, err := net.ResolveUDPAddr("udp", "0.0.0.0:1234")
	if err != nil {
		log.Fatal("failed to resolve address: ", err)
	}
	conn, err := net.ListenUDP("udp", addr)
	if err != nil {
		log.Fatal("failed to start listening: ", err)
	}
	defer conn.Close()

	mode := &serial.Mode{
		BaudRate: 9600,
		DataBits: 8,
		Parity:   serial.NoParity,
		StopBits: serial.OneStopBit,
	}
	port, err := serial.Open("/dev/ttyS0", mode)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
	}
	defer port.Close()

	udpChan, addrChan := udpRecv(conn)
	serialChan := serialRecv(port)
	imuChan := imu()

	var clientAddr *net.UDPAddr = nil
	for {
		select {
		case addr := <-addrChan:
			clientAddr = addr
		case udpPacket := <-udpChan:
			n, err := port.Write([]byte(udpPacket))
			if err != nil {
				fmt.Fprintf(os.Stderr, "failed to write to serial port: %s\n", err)
			} else {
				fmt.Printf("wrote %d bytes to the serial port:\n", n)
				fmt.Printf("udpPacket = %s\n", udpPacket)
			}
		case serialMsg := <-serialChan:
			if clientAddr != nil {
				fmt.Printf("received from uart: %s\n", serialMsg)
				conn.WriteToUDP([]byte(serialMsg), clientAddr)
			}
		case imuMsg := <-imuChan:
			if clientAddr != nil {
				conn.WriteToUDP([]byte(imuMsg), clientAddr)
			}
		}
	}
}

func udpRecv(conn *net.UDPConn) (<-chan string, chan *net.UDPAddr) {
	msgChan := make(chan string, 10)
	addrChan := make(chan *net.UDPAddr)
	go func() {
		buffer := make([]byte, 1500)
		for {
			n, clientAddr, err := conn.ReadFromUDP(buffer)
			if err != nil {
				fmt.Fprintln(os.Stderr, err)
				continue
			}
			fmt.Printf("received %s\n", buffer[:n])
			addrChan <- clientAddr
			msgChan <- string(buffer[:n])
		}
	}()
	return msgChan, addrChan
}

func serialRecv(port serial.Port) <-chan string {
	c := make(chan string, 10)
	scanner := bufio.NewScanner(port)
	go func() {
		for scanner.Scan() {
			c <- scanner.Text()
		}
	}()
	return c
}

func imu() <-chan string {
	c := make(chan string, 10)
	go func() {
		if _, err := host.Init(); err != nil {
			fmt.Fprintln(os.Stderr, err)
			return
		}
		b, err := i2creg.Open("1")
		if err != nil {
			fmt.Fprintln(os.Stderr, err)
			return
		}
		defer b.Close()
		d := &i2c.Dev{Addr: 0x60, Bus: b}

		tx := []byte{0x2}
		rx := make([]byte, 6)
		for {
			err = d.Tx(tx, rx)
			if err != nil {
				fmt.Fprintln(os.Stderr, err)
				time.Sleep(1 * time.Second)
				continue
			}
			heading := float32((int(rx[0])<<8)+int(rx[1])) / 10
			pitch := float32(i8(rx[2]))
			roll := float32(i8(rx[3]))
			c <- fmt.Sprintf("#heading,%f!", heading)
			c <- fmt.Sprintf("#pitch,%f!", pitch)
			c <- fmt.Sprintf("#roll,%f!", roll)
			time.Sleep(100 * time.Millisecond)
		}
	}()
	return c
}

func i8(x byte) int8 {
	if x > 127 {
		return int8(int(x) - 256)
	} else {
		return int8(x)
	}
}
