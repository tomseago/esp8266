package main

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"net"
	"os"
	"strconv"
	"time"
)

var pieceNum = 0
var numPixels = 2
var bytesPerPixel = 3

////////////////
// Color stuff

func rgbFromHSB(h float32, s float32, v float32) (byte, byte, byte) {
	var r float32
	var g float32
	var b float32

	if s == 0.0 {
		// black, leave at 0 values
	} else {
		// clamp
		if h < 0.0 {
			h += 1.0
		} else if h > 1.0 {
			h -= 1.0
		}
		h *= 6.0

		i := int(h)
		f := h - float32(i)
		q := v * (1.0 - s*f)
		p := v * (1.0 - s)
		t := v * (1.0 - s*(1.0-f))

		switch i {
		case 0:
			r = v
			g = t
			b = p

		case 1:
			r = q
			g = v
			b = p

		case 2:
			r = p
			g = v
			b = t

		case 3:
			r = p
			g = q
			b = v

		case 4:
			r = t
			g = p
			b = v

		default:
			r = v
			g = p
			b = q
		}
	}

	R := uint8(r * 255.0)
	G := uint8(g * 255.0)
	B := uint8(b * 255.0)

	return R, G, B
}

////////////////

func send(c net.Conn, buf []byte) error {
	fmt.Printf("> %+v\n", buf)

	_, err := c.Write(buf)
	if err != nil {
		return err
	}

	return err

}

////

func addLabel(buf *bytes.Buffer, tag byte, label string) error {
	err := buf.WriteByte(tag)
	if err != nil {
		return err
	}

	_, err = buf.WriteString(label)
	if err != nil {
		return err
	}

	err = buf.WriteByte(0)
	if err != nil {
		return err
	}

	return nil
}

func sendSetup(c net.Conn) error {
	buf := new(bytes.Buffer)

	// Message Type 2
	// uint16 pieceNumber
	// uint16 numPixels
	// uint8  bytesPerPixel
	err := buf.WriteByte(2)
	if err != nil {
		return err
	}

	err = binary.Write(buf, binary.BigEndian, uint16(pieceNum))
	if err != nil {
		return err
	}

	err = binary.Write(buf, binary.BigEndian, uint16(numPixels))
	if err != nil {
		return err
	}

	err = buf.WriteByte(byte(bytesPerPixel))
	if err != nil {
		return err
	}

	err = send(c, buf.Bytes())
	if err != nil {
		return err
	}

	// Message Type 3 labels
	// uint8   state variable tag, 0 indicates end of message
	// string  label
	buf.Truncate(0)

	err = buf.WriteByte(3)
	if err != nil {
		return err
	}

	err = addLabel(buf, 1, "First")
	if err != nil {
		return err
	}

	err = addLabel(buf, 2, "Second")
	if err != nil {
		return err
	}

	// end of label array
	err = buf.WriteByte(0)
	if err != nil {
		return err
	}

	err = send(c, buf.Bytes())
	if err != nil {
		return err
	}

	return nil
}

////

var pixels []byte

func setupPixels() {
	pixels = make([]byte, numPixels*3)
}

func updatePixels(c net.Conn) error {
	animate()

	err := sendValues(c)
	if err != nil {
		return err
	}

	err = sendPixels(c)
	if err != nil {
		return err
	}

	return nil
}

var loopDuration = int64(2 * time.Second)

func animate() {
	now := time.Now().UnixNano()
	position := float32(now%loopDuration) / float32(loopDuration)

	hsbRainbowAnim(position)
}

func hsbRainbowAnim(position float32) {
	for i := 0; i < numPixels; i++ {
		colorPos := position + (float32(i) / float32(numPixels))
		r, g, b := rgbFromHSB(colorPos, 1.0, 1.0)
		setPixel(i, r, g, b)
	}
}

func setPixel(ix int, r byte, g byte, b byte) {
	subIx := ix * int(bytesPerPixel)

	pixels[subIx] = r
	pixels[subIx+1] = g
	pixels[subIx+2] = b
}

func sendPixels(c net.Conn) error {
	buf := new(bytes.Buffer)

	// Message type 1, pixel data
	err := buf.WriteByte(1)
	if err != nil {
		return err
	}

	_, err = buf.Write(pixels)
	if err != nil {
		return err
	}

	// Send it
	send(c, buf.Bytes())
	return fmt.Errorf("Forcing a stop")
}

func addStringVal(buf *bytes.Buffer, tag byte, s string) error {
	err := buf.WriteByte(tag)
	if err != nil {
		return err
	}

	err = buf.WriteByte(0) // type string
	if err != nil {
		return err
	}

	_, err = buf.WriteString(s)
	if err != nil {
		return err
	}

	err = buf.WriteByte(0)
	if err != nil {
		return err
	}

	return nil
}

func addIntVal(buf *bytes.Buffer, tag byte, v int32) error {
	err := buf.WriteByte(tag)
	if err != nil {
		return err
	}

	err = buf.WriteByte(1) // type int
	if err != nil {
		return err
	}

	err = binary.Write(buf, binary.BigEndian, v)
	if err != nil {
		return err
	}

	return nil
}

func addFloatVal(buf *bytes.Buffer, tag byte, v float32) error {
	err := buf.WriteByte(tag)
	if err != nil {
		return err
	}

	err = buf.WriteByte(2) // type float
	if err != nil {
		return err
	}

	err = binary.Write(buf, binary.BigEndian, v)
	if err != nil {
		return err
	}

	return nil
}

func sendValues(c net.Conn) error {
	buf := new(bytes.Buffer)

	// Message type 4, value data
	err := buf.WriteByte(4)
	if err != nil {
		return err
	}

	err = addStringVal(buf, 1, "Str")
	if err != nil {
		return err
	}

	err = addIntVal(buf, 2, 874)
	if err != nil {
		return err
	}

	err = addFloatVal(buf, 3, 12.34)
	if err != nil {
		return err
	}

	// end of value array
	err = buf.WriteByte(0)
	if err != nil {
		return err
	}

	err = send(c, buf.Bytes())
	if err != nil {
		return err
	}

	return nil
}

////

func mainWithCode() int {

	// Read command line arg
	args := os.Args[1:]
	if len(args) > 0 {
		pieceNum, _ = strconv.Atoi(args[0])
	}
	fmt.Printf("Piece #%v\n", pieceNum)

	setupPixels()

	conn, err := net.Dial("tcp", "127.0.0.1:7453")
	if err != nil {
		fmt.Printf("ERROR: Dial %v\n", err)
		return -1
	}
	defer conn.Close()

	// Do this before messages
	sendSetup(conn)

	fmt.Printf("Starting ticker...\n")
	ticker := time.NewTicker(time.Millisecond * 500)

	// go func() {
	// 	err := readAddr(conn)
	// 	if err != nil {
	// 		ticker.Stop()
	// 		return
	// 	}

	// 	for err == nil {
	// 		err = readMsg(conn)
	// 	}

	// 	ticker.Stop()
	// }()

	go func() {
		for _ = range ticker.C {
			err := updatePixels(conn)
			if err != nil {
				fmt.Printf("ERROR. Stopping. : %v\n", err)
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
