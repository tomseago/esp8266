package main

import (
	"fmt"
	"io/ioutil"
	"net"
	"net/http"
	"os"
	"strconv"
	"time"
)

////////

const TypeUInt8 = 1
const TypeUInt16 = 2
const TypeUInt32 = 3
const TypeString = 8
const TypeBytes = 9

const TagVersion = 1
const TagURL = 2

type TaggedBuffer struct {
	data []byte
}

func NewTaggedBuffer() *TaggedBuffer {
	tb := &TaggedBuffer{
		data: make([]byte, 0),
	}

	return tb
}

func (tb *TaggedBuffer) writeUInt8(tag uint8, v uint8) int {
	tb.data = append(tb.data, tag)
	tb.data = append(tb.data, TypeUInt8)
	tb.data = append(tb.data, v)

	return 3
}

func (tb *TaggedBuffer) writeUInt32(tag uint8, v uint32) int {
	tb.data = append(tb.data, tag)
	tb.data = append(tb.data, TypeUInt32)
	tb.data = append(tb.data, byte((v>>24)&0x00ff))
	tb.data = append(tb.data, byte((v>>16)&0x00ff))
	tb.data = append(tb.data, byte((v>>8)&0x00ff))
	tb.data = append(tb.data, byte((v)&0x00ff))

	return 6
}

func (tb *TaggedBuffer) writeString(tag uint8, str string) int {
	start := len(tb.data)

	tb.data = append(tb.data, tag)
	tb.data = append(tb.data, TypeString)
	slen := len(str)
	tb.data = append(tb.data, byte(slen))

	tb.data = append(tb.data, []byte(str)...)

	return len(tb.data) - start
}

func (tb *TaggedBuffer) writeNull() int {
	tb.data = append(tb.data, 0)

	return 1
}

////////

var filename string
var versionNum uint32

var firmware []byte

func sendFirmware(w http.ResponseWriter, req *http.Request) {

	fmt.Printf("Got firmware request from %v\n", req.RemoteAddr)
	fmt.Printf("Headers: %v\n", req.Header)

	mac := req.Header.Get("X-Esp8266-Sta-Mac")

	node := req.Header.Get("x-ESP8266-version")
	fmt.Printf(" --->  %v  ==  %v\n", mac, node)

	w.Header().Set("content-type", "application/binary")
	w.Header().Set("content-length", fmt.Sprintf("%d", len(firmware)))

	sent, err := w.Write(firmware)
	if err != nil {
		fmt.Printf("Failed to send firmware: %v\n", err)
		return
	}

	fmt.Printf("Sent firmware to %v  i=%v  mac=%v  node=%v\n", req.RemoteAddr, sent, mac, node)
}

var myip *net.IPNet

var bcast *TaggedBuffer

func buildBroadcast() {
	addrs, err := net.InterfaceAddrs()
	if err != nil {
		os.Stderr.WriteString("Oops: " + err.Error() + "\n")
		os.Exit(1)
	}

	for _, a := range addrs {
		if ipnet, ok := a.(*net.IPNet); ok && !ipnet.IP.IsLoopback() {
			if ipnet.IP.To4() != nil {
				fmt.Printf("Found address %v\n", ipnet.IP.String())
				myip = ipnet
			}
		}
	}

	bcast = NewTaggedBuffer()

	bcast.writeUInt32(TagVersion, versionNum)
	url := "http://" + myip.IP.String() + ":7454/fw.bin"
	fmt.Printf("url = %v\n", url)
	bcast.writeString(TagURL, url)

	bcast.writeNull()

	fmt.Printf("bcast data = ")
	for i := 0; i < len(bcast.data); i++ {
		fmt.Printf("%02x ", bcast.data[i])
	}
	fmt.Printf("\n")
}

func sendBroadcast() {
	fmt.Printf("Sending a broadcast about the firmware\n")

	raddr, err := net.ResolveUDPAddr("udp", "255.255.255.255:7777")
	if err != nil {
		fmt.Printf("Can't resolve addr: %v\n", err)
		return
	}

	udp, err := net.DialUDP("udp", nil, raddr)
	if err != nil {
		fmt.Printf("UDP Dial failed: %v\n", err)
		return
	}

	_, err = udp.Write(bcast.data)
	if err != nil {
		fmt.Printf("Failed to send broadcast: %v\n")
		return
	}

	udp.Close()

	fmt.Printf("Broadcast sent\n")
}

func mainWithCode() int {

	// Read command line arg
	args := os.Args[1:]

	if len(args) < 2 {
		fmt.Printf("Must provide a firmware filename and an integer version number")
	}

	filename = args[0]
	num, _ := strconv.ParseUint(args[1], 10, 32)
	versionNum = uint32(num)

	// Read the file
	var err error
	firmware, err = ioutil.ReadFile(filename)
	if err != nil {
		fmt.Printf("Unable to read %v : %v\n", filename, err)
		return -1
	}

	buildBroadcast()

	// Start telling people about it
	fmt.Printf("Starting ticker to tell people about it...\n")
	ticker := time.NewTicker(time.Millisecond * 5000)

	go func() {
		for _ = range ticker.C {
			sendBroadcast()
		}
	}()

	http.HandleFunc("/fw.bin", sendFirmware)
	listener, err := net.Listen("tcp4", ":7454")
	if err != nil {
		fmt.Printf("Error with Listen: %v\n", err)
		return -1
	}
	http.Serve(listener, nil)
	fmt.Printf("After Listen and Serve\n")

	time.Sleep(time.Hour * 1)
	ticker.Stop()
	fmt.Println("Stopping based on timeout")
	return 0
}

func main() {
	os.Exit(mainWithCode())
}

func mainX() {
	tb := NewTaggedBuffer()

	tb.writeUInt32(1, 0xfeedface)
	tb.writeString(2, "abcdefg")

	fmt.Printf("%v\n", tb.data)
}
