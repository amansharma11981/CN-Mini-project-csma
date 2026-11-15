# CSMA, CSMA/CD, and CSMA/CA Protocol Simulator in C++

An educational Computer Networks mini-project demonstrating **Carrier Sense Multiple Access (CSMA)** protocols in modern, beginner-friendly standard C++.

---

## Project Structure

This project contains two core files:
- `csma_simulation.cpp` – Standalone C++ source code containing all three protocol simulations and interactive user configuration.
- `README.md` – Documentation, theoretical principles, and usage instructions.

---

## Features & User Control

### 1. Pure CSMA (1-Persistent)
- **Concept**: Sense medium; if idle, transmit immediately ($p = 1$). If busy, continuously listen and transmit immediately when free.
- **User Control**: Number of competing stations ($2$ to $6$).
- **Demonstration**: Shows how stations queueing up during a busy transmission collide as soon as the channel frees.

### 2. CSMA/CD (Collision Detection — IEEE 802.3 Ethernet)
- **Concept**: Listen-while-talk, instant transmission abortion, 32-bit Jam signal, and Truncated Binary Exponential Backoff ($R \in [0, 2^k - 1]$).
- **User Control**: Number of contending stations ($2$ to $6$).
- **Demonstration**: Shows simultaneous transmission, collision detection, jamming, and how backoff slots desynchronize nodes.

### 3. CSMA/CA (Collision Avoidance — IEEE 802.11 Wi-Fi)
- **Concept**: Avoid-before-talk, DIFS sensing, Contention Window backoff countdown, simplified RTS/CTS reservation, SIFS, and ACK confirmation.
- **User Control**:
  - Number of wireless stations ($2$ to $5$).
  - Enable / Disable RTS/CTS reservation handshake ($1$ = Yes, $0$ = No).
- **Demonstration**: Shows how small RTS/CTS frames protect large data frames from collision and address the hidden-terminal problem using NAV.

---

## How to Compile & Run

Compile with any standard C++ compiler:
```bash
g++ csma_simulation.cpp -o csma_sim
./csma_sim
```
*(On Windows: `.\csma_sim.exe`)*

---

## Interactive Menu Overview

```text
======================================================
           CSMA / CD / CA CONSOLE SIMULATOR           
======================================================
1. Simulate Pure CSMA (1-Persistent)
2. Simulate CSMA/CD (Collision Detection)
3. Simulate CSMA/CA (Collision Avoidance & RTS/CTS)
4. Exit
Enter your choice [1-4] (default 1):
```

At any prompt, you can press **Enter** to accept the sensible default value or type a custom value to test different network conditions.
