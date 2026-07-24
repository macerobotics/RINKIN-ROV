package main

import (
	"fmt"
	"log"
	"net"
	"os"
	"simulation/rov"
	"time"
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

	var clientAddr *net.UDPAddr = nil

	udpChan, addrChan := udpRecv(conn)

	var info rov.ROVInfo

	rov := rov.New()

	for {
		select {
		case addr := <-addrChan:
			clientAddr = addr
			rov.Start()
		case udpPacket := <-udpChan:
			fmt.Println("received command: ", udpPacket)
			rov.Cmd <- udpPacket
		case info = <-rov.Info:
		case <-time.After(16 * time.Millisecond):

		}
		if clientAddr != nil {
			msg := fmt.Sprintf("#pos_x,%f!", info.Position[0])
			conn.WriteToUDP([]byte(msg), clientAddr)
			msg = fmt.Sprintf("#pos_y,%f!", info.Position[1])
			conn.WriteToUDP([]byte(msg), clientAddr)
			msg = fmt.Sprintf("#pos_z,%f!", info.Position[2])
			conn.WriteToUDP([]byte(msg), clientAddr)
			msg = fmt.Sprintf("#heading,%f!", info.Orientation.Heading)
			conn.WriteToUDP([]byte(msg), clientAddr)
			msg = fmt.Sprintf("#pitch,%f!", info.Orientation.Pitch)
			conn.WriteToUDP([]byte(msg), clientAddr)
			msg = fmt.Sprintf("#roll,%f!", info.Orientation.Roll)
			conn.WriteToUDP([]byte(msg), clientAddr)
			fmt.Println("info: ", info)
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
