package main

import (
	"bufio"
	"fmt"
	"log"
	"net"
	"os"
	"regexp"
	"strconv"
	"time"

	"go.bug.st/serial"
	"periph.io/x/conn/v3/i2c"
	"periph.io/x/conn/v3/i2c/i2creg"
	"periph.io/x/host/v3"
)

var delay = 35 * time.Millisecond

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
		BaudRate: 115200,
		DataBits: 8,
		Parity:   serial.NoParity,
		StopBits: serial.OneStopBit,
	}
	//port, err := serial.Open("/dev/ttyAMA0", mode)
	port, err := serial.Open("/dev/ttyS0", mode)
	if err != nil {
		fmt.Fprintln(os.Stderr, err)
	}
	defer port.Close()

	udpChan, addrChan := udpRecv(conn)
	serialRxChan := serialRx(port)
	serialTxChan := serialTx(port)
	imuChan := imu()

	var clientAddr *net.UDPAddr = nil

	for {
		select {
		case addr := <-addrChan:
			clientAddr = addr
		case udpPacket := <-udpChan:
			fmt.Printf("pc\t->\trpi\t%s\n", strconv.Quote(udpPacket))
			serialTxChan <- udpPacket
		case serialMsg := <-serialRxChan:
			fmt.Printf("pico\t->\trpi\t%s\n", strconv.Quote(serialMsg))
			if clientAddr != nil {
				serialMsg = fmt.Sprintf("#battery,%s!", serialMsg)
				fmt.Printf("rpi\t->\tpc\t%s\n", strconv.Quote(serialMsg))
				conn.WriteToUDP([]byte(serialMsg), clientAddr)
			}
		case imuMsg := <-imuChan:
			if clientAddr != nil {
				fmt.Printf("rpi\t->\tpc\t%s\n", strconv.Quote(imuMsg))
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
			msg := string(buffer[:n])
			addrChan <- clientAddr
			msgChan <- string(msg)
		}
	}()
	return msgChan, addrChan
}

func serialRx(port serial.Port) <-chan string {
	c := make(chan string, 10)
	scanner := bufio.NewScanner(port)
	go func() {
		for scanner.Scan() {
			c <- scanner.Text()
		}
	}()
	return c
}

func serialTx(port serial.Port) chan<- string {
	c1 := make(chan string, 1024)
	c2 := make(chan string)
	go func() {
		for {
			s := <-c2
			_, err := port.Write([]byte(s))
			if err != nil {
				fmt.Fprintf(os.Stderr, "failed to write to serial port: %s\n", err)
			} else {
				fmt.Printf("rpi\t->\tpico\t%s\n", strconv.Quote(s))
			}
			time.Sleep(delay)
		}
	}()

	go func() {
		r, _ := regexp.Compile(`#(\d+[mlb])(-*\d+)!\n`)
		q := NewQueue()
		for {
		loop1:
			for {
				select {
				case s := <-c1:
					l := r.FindStringSubmatch(s)
					if len(l) == 3 {
						name := l[1]
						value := l[2]
						q.Push(name, value)
					}
				default:
					break loop1
				}
			}
			if s := q.Pop(); s != nil {
				c2 <- fmt.Sprintf("#%s%s!\n", s.slot, s.value)
			}
		}
	}()

	return c1
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
