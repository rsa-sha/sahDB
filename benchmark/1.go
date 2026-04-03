package main

import (
	"bufio"
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
}

func randomValue(size int) string {
	b := make([]byte, size)
	for i := range b {
		b[i] = byte('a' + rand.Intn(26))
	}
	return string(b)
}

func sendCommand(conn net.Conn, cmd string) (string, error) {
	_, err := conn.Write([]byte(cmd + "\n"))
	if err != nil {
		return "", err
	}

	reader := bufio.NewReader(conn)
	resp, err := reader.ReadString('\n')
	return resp, err
}

func worker(id int, cfg Config, wg *sync.WaitGroup, results chan time.Duration) {
	defer wg.Done()

	conn, err := net.Dial("tcp", cfg.Addr)
	if err != nil {
		fmt.Println("Connection error:", err)
		return
	}
	defer conn.Close()

	for i := 0; i < cfg.NumOps/cfg.Concurrency; i++ {
		key := "key_" + strconv.Itoa(rand.Intn(100000))
		start := time.Now()

		if rand.Float64() < cfg.GetRatio {
			sendCommand(conn, "GET "+key)
		} else {
			val := randomValue(cfg.ValueSize)
			sendCommand(conn, "SET "+key+" "+val)
		}

		latency := time.Since(start)
		results <- latency
	}
}

func main() {
	cfg := Config{
		Addr:        "localhost:5555",
		NumOps:      100000,
		ValueSize:   128,
		GetRatio:    0.8,
		Concurrency: 10,
	}

	rand.Seed(time.Now().UnixNano())

	var wg sync.WaitGroup
	results := make(chan time.Duration, cfg.NumOps)

	start := time.Now()

	for i := 0; i < cfg.Concurrency; i++ {
		wg.Add(1)
		go worker(i, cfg, &wg, results)
	}

	wg.Wait()
	close(results)

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

	avgLatency := totalLatency / time.Duration(count)
	throughput := float64(count) / totalDuration.Seconds()

	fmt.Println("---- Benchmark Results ----")
	fmt.Println("Total Ops:", count)
	fmt.Println("Total Time:", totalDuration)
	fmt.Println("Throughput (ops/sec):", throughput)
	fmt.Println("Avg Latency:", avgLatency)
	fmt.Println("Max Latency:", maxLatency)
}
