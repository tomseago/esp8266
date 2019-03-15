package main

import (
	"bufio"
	"bytes"
	"encoding/binary"
	"fmt"
	"math/rand"
	"net"
	"os"
	"time"
)

type Message struct {
	id       uint32
	src      byte
	dest     byte
	typeCode byte

	data []byte
}

var lastId uint32 = 0

func NewMessage() *Message {
	lastId += 1

	return &Message{
		id: lastId,
	}
}

func (m *Message) send(conn net.Conn) error {
	fmt.Printf("sendMsg\n")
	buf := new(bytes.Buffer)

	// id
	err := binary.Write(buf, binary.LittleEndian, m.id)
	if err != nil {
		return err
	}

	// src
	err = binary.Write(buf, binary.LittleEndian, m.src)
	if err != nil {
		return err
	}

	// dest
	err = binary.Write(buf, binary.LittleEndian, m.dest)
	if err != nil {
		return err
	}

	// typeCode
	err = binary.Write(buf, binary.LittleEndian, m.typeCode)
	if err != nil {
		return err
	}

	if len(m.data) > 0 {
		// data length
		err = binary.Write(buf, binary.LittleEndian, uint16(len(m.data)))
		if err != nil {
			return err
		}

		_, err = buf.Write(m.data)
		if err != nil {
			return err
		}
	} else {
		err = binary.Write(buf, binary.LittleEndian, uint16(0))
		if err != nil {
			return err
		}
	}

	// // Data
	// err = binary.Write(buf, binary.LittleEndian, byte(1))
	// if err != nil {
	// 	return err
	// }

	// Output it...
	_, err = conn.Write(buf.Bytes())
	if err != nil {
		return err
	}

	fmt.Printf("SENT ---> %+v\n", m)
	return err
}

func (m *Message) read(reader *bufio.Reader) error {
	return nil
}

////////////////////

var localAddr byte = 200
var remoteAddr byte = 255

func sendAddr(c net.Conn) error {
	out := []byte{localAddr}
	_, err := c.Write(out)
	return err
}

func sendMsg(c net.Conn) error {
	m := NewMessage()
	m.src = localAddr
	m.dest = remoteAddr

	m.data = []byte("Some data here.")

	return m.send(c)
}

////

func read8(c net.Conn) (byte, error) {
	buf := make([]byte, 1)
	_, err := c.Read(buf)
	if err != nil {
		return 0, err
	}
	return buf[0], nil
}

func read16(c net.Conn) (uint16, error) {

	var v uint16
	err := binary.Read(c, binary.LittleEndian, &v)
	if err != nil {
		return 0, err
	}

	return v, nil
}

func read32(c net.Conn) (uint32, error) {

	var v uint32
	err := binary.Read(c, binary.LittleEndian, &v)
	if err != nil {
		return 0, err
	}

	return v, nil
}

func readAddr(c net.Conn) error {
	v, err := read8(c)
	if err != nil {
		return err
	}
	remoteAddr = v
	fmt.Printf("Remote Address: %v\n", remoteAddr)

	return nil
}

func readMsg(c net.Conn) error {
	m := NewMessage()

	var err error

	m.id, err = read32(c)
	if err != nil {
		return err
	}

	m.src, err = read8(c)
	if err != nil {
		return err
	}

	m.dest, err = read8(c)
	if err != nil {
		return err
	}

	m.typeCode, err = read8(c)
	if err != nil {
		return err
	}

	var mlen uint16
	mlen, err = read16(c)
	if err != nil {
		return err
	}
	if mlen > 0 {
		m.data = make([]byte, mlen)
		_, err := c.Read(m.data)
		if err != nil {
			return err
		}
	}

	fmt.Printf("RECV <--- %+v\n", m)
	return nil
}

////

func mainWithCode() int {
	rand.Seed(time.Now().Unix())
	localAddr = 100 + byte(rand.Intn(100))

	conn, err := net.Dial("tcp", "10.7.10.10:2000")
	if err != nil {
		fmt.Printf("ERROR: Dial %v\n", err)
		return -1
	}
	defer conn.Close()

	// Do this before messages
	sendAddr(conn)

	fmt.Printf("Starting ticker...\n")
	ticker := time.NewTicker(time.Millisecond * 500)

	go func() {
		err := readAddr(conn)
		if err != nil {
			ticker.Stop()
			return
		}

		for err == nil {
			err = readMsg(conn)
		}

		ticker.Stop()
	}()

	go func() {
		for _ = range ticker.C {
			err := sendMsg(conn)
			if err != nil {
				ticker.Stop()
			}
		}
	}()

	time.Sleep(time.Hour * 1)
	ticker.Stop()
	fmt.Println("Stopping based on timeout")
	return 0
}

func main() {
	os.Exit(mainWithCode())
}
