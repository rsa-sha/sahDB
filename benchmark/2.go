package main

import (
	"fmt"
	"math/rand"
	"net"
	"strconv"
	"sync"
	"time"
)

type Config struct {
	Addr        string
	NumOps      int
	ValueSize   int
	GetRatio    float64
	Concurrency int
	KeySpace    int
	Mode        string
	Preload     int
}

func randomValue(size int) string {
	b := make([]byte, size)
	for i := range b {
		b[i] = byte('a' + rand.Intn(26))
	}
	return string(b)
}

// fire-and-drain with timeout
func sendCommand(conn net.Conn, cmd string) error {
	_, err := conn.Write([]byte(cmd + "\n"))
	if err != nil {
		return err
	}

	// try to read response for max 50ms (non-blocking style)
	buf := make([]byte, 1024)
	conn.SetReadDeadline(time.Now().Add(1 * time.Millisecond))

	for {
		_, err := conn.Read(buf)
		if err != nil {
			break // timeout or no more data → fine
		}
	}

	return nil
}

func connect(addr string) (net.Conn, error) {
	conn, err := net.Dial("tcp", addr)
	if err != nil {
		return nil, err
	}

	// send handshake but don't wait
	conn.Write([]byte("USER_JOIN\n"))

	return conn, nil
}

func preload(cfg Config) {
	fmt.Println("Preloading dataset...")

	conn, err := connect(cfg.Addr)
	if err != nil {
		fmt.Println("Preload connection error:", err)
		return
	}
	defer conn.Close()

	for i := 0; i < cfg.Preload; i++ {
		key := "key_" + strconv.Itoa(i)
		val := randomValue(cfg.ValueSize)

		err := sendCommand(conn, "SET "+key+" "+val)
		if err != nil {
			fmt.Println("Error at key:", i, err)
			return
		}

		if i%1000 == 0 {
			fmt.Println("Inserted:", i)
		}
	}

	fmt.Println("Preload complete:", cfg.Preload)
}

func worker(cfg Config, wg *sync.WaitGroup, results chan time.Duration, errors chan int) {
	defer wg.Done()

	conn, err := connect(cfg.Addr)
	if err != nil {
		fmt.Println("Connection error:", err)
		return
	}
	defer conn.Close()

	opsPerWorker := cfg.NumOps / cfg.Concurrency

	for i := 0; i < opsPerWorker; i++ {
		key := "key_" + strconv.Itoa(rand.Intn(cfg.KeySpace))

		start := time.Now()

		switch cfg.Mode {
		case "set":
			val := randomValue(cfg.ValueSize)
			err := sendCommand(conn, "SET "+key+" "+val)
			if err != nil {
				errors <- 1
			}

		case "get":
			err := sendCommand(conn, "GET "+key)
			if err != nil {
				errors <- 1
			}

		case "mixed":
			if rand.Float64() < cfg.GetRatio {
				err := sendCommand(conn, "GET "+key)
				if err != nil {
					errors <- 1
				}
			} else {
				val := randomValue(cfg.ValueSize)
				err := sendCommand(conn, "SET "+key+" "+val)
				if err != nil {
					errors <- 1
				}
			}
		}

		results <- time.Since(start)
	}
}

func main() {
	cfg := Config{
		Addr:        "localhost:5555",
		NumOps:      100000,
		ValueSize:   128,
		GetRatio:    0.8,
		Concurrency: 10,
		KeySpace:    10000,
		Mode:        "mixed",
		Preload:     10000,
	}

	rand.Seed(time.Now().UnixNano())

	if cfg.Mode == "get" || cfg.Mode == "mixed" {
		preload(cfg)
	}

	var wg sync.WaitGroup
	results := make(chan time.Duration, cfg.NumOps)
	errors := make(chan int, cfg.NumOps)

	start := time.Now()

	for i := 0; i < cfg.Concurrency; i++ {
		wg.Add(1)
		go worker(cfg, &wg, results, errors)
	}

	wg.Wait()
	close(results)
	close(errors)

	totalDuration := time.Since(start)

	var count int
	var totalLatency time.Duration
	var maxLatency time.Duration

	for lat := range results {
		totalLatency += lat
		if lat > maxLatency {
			maxLatency = lat
		}
		count++
	}

	errorCount := 0
	for range errors {
		errorCount++
	}

	fmt.Println("---- Benchmark Results ----")
	fmt.Println("Mode:", cfg.Mode)
	fmt.Println("Total Ops:", count)
	fmt.Println("Errors:", errorCount)
	fmt.Println("Total Time:", totalDuration)
	fmt.Println("Throughput (ops/sec):", float64(count)/totalDuration.Seconds())
	fmt.Println("Avg Latency:", totalLatency/time.Duration(count))
	fmt.Println("Max Latency:", maxLatency)
}
