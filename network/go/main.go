package main


/*
#cgo CFLAGS: -I../../include
#cgo LDFLAGS: -L../../build -lkv
#include <stdlib.h>
#include "common.h"
#include "config.h"
#include "db.h"
#include "hash.h"
// Add forward declarations (if needed for cgo symblol discovery)
extern void send_info_to_user(const char *data);
extern int get_user_input(char *buf);
extern int processCommand(char *req);
extern int getAndProcessCommand();


*/
import "C"

import (
	"fmt"
	"net"
	"os"
	"unsafe"
)


const SIG_EXIT = 41

func run_db_server(port string) {

	listener, err := net.Listen("tcp", ":"+port)  // Capital L in Listen

	if err != nil {
		fmt.Println("Failed to listen:", err)
		return
	}

	defer listener.Close()

	fmt.Println("Listening on port", port)

	var resp [1024]byte  // Allocate a buffer for C.get_user_input to write into

	for {
		conn, err := listener.Accept()
		if err != nil {
			fmt.Println("Accept error:", err)
			continue
		}
		fmt.Println("Client connected:", conn.RemoteAddr())

		tcpConn, ok := conn.(*net.TCPConn)
		if !ok {
			fmt.Println("Connection is not TCP")
			conn.Close()
			continue
		}

		file, err := tcpConn.File()
		if err != nil {
			fmt.Println("Failed to get fd:", err)
			conn.Close()
			continue
		}
		fd := int(file.Fd())
		fmt.Println("Client fd:", fd)

		C.init_app_config(C.int(fd), C.int(fd))

		for {
			result := int(C.get_user_input((*C.char)(unsafe.Pointer(&resp[0])))) // pass pointer to buffer

			if result != 0 {
				break
			}

			result = int(C.processCommand((*C.char)(unsafe.Pointer(&resp[0]))))

			if result == SIG_EXIT {
				break
			}
		}

		file.Close()
		conn.Close()
		break // If you want to handle only one connection, else remove this line
	}
}


func event_loop() {
	res := 0
	for {
		if res == SIG_EXIT {
			break
		}
		res = int(C.getAndProcessCommand())
	}
	return;
}


func main() {
	args := os.Args

	C.ht_init()

	if len(args) == 3 {
		run_db_server(args[2])
	} else {
		event_loop()
	}
	fmt.Println("Go calling C static lib...")

	msg := C.CString("Hello from Go!")
	defer C.free(unsafe.Pointer(msg))

	C.send_info_to_user(msg)

	fmt.Println("Call complete.")
}