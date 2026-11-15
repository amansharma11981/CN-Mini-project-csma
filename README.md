# CSMA, CSMA/CD, and CSMA/CA Simulation in Pure C

An educational Computer Networks mini-project demonstrating **Carrier Sense Multiple Access (CSMA)** protocols in pure C with **complete user control** over simulation parameters.

---

## Project Structure

This project contains only two files:
- `csma_simulation.c` – Standalone C source code containing all three protocol simulations, interactive user configuration, and an automated demo runner.
- `README.md` – Documentation, theoretical principles, and usage instructions.

---

## Features & User Control

### 1. Pure CSMA (1-Persistent)
- **Concept**: Sense medium; if idle, transmit immediately ($p = 1$). If busy, continuously listen and transmit immediately when free.
- **User Control**:
  - Number of competing stations ($2$ to $6$).
  - Transmission duration in time slots.
  - Optional step-by-step mode.
- **Demonstration**: Shows how stations queueing up during a busy transmission collide as soon as the channel frees.

### 2. CSMA/CD (Collision Detection — IEEE 802.3 Ethernet)
- **Concept**: Listen-while-talk, instant transmission abortion, 32-bit Jam signal, and Truncated Binary Exponential Backoff ($R \in [0, 2^k - 1]$).
- **User Control**:
  - Number of contending stations ($2$ to $6$).
  - Max collision retry attempts ($3$ to $16$).
  - Slot time in milliseconds.
  - Optional step-by-step mode.
- **Demonstration**: Shows simultaneous transmission, collision detection, jamming, and how backoff slots desynchronize nodes.

### 3. CSMA/CA (Collision Avoidance — IEEE 802.11 Wi-Fi)
- **Concept**: Avoid-before-talk, DIFS sensing, Contention Window backoff countdown, simplified RTS/CTS reservation, SIFS, and ACK confirmation.
- **User Control**:
  - Number of wireless stations ($2$ to $5$).
  - Contention Window size ($CW \in [4, 32]$).
  - Enable / Disable RTS/CTS reservation handshake.
  - Optional step-by-step mode.
- **Demonstration**: Shows how small RTS/CTS frames protect large data frames from collision and address the hidden-terminal problem using NAV.

### 4. Automated All-in-One Demo
- Runs all three protocols sequentially using standard defaults for quick demonstration and laboratory evaluation.

---

## How to Compile & Run

### Compile
Compile with any standard C99/C11 compiler:
```bash
gcc csma_simulation.c -o csma_sim
```

### Run Interactive Menu
```powershell
.\csma_sim.exe
```
*(On Linux/macOS: `./csma_sim`)*

### Run Automated Demo Directly
```powershell
.\csma_sim.exe --demo
```

---

## Interactive Menu Overview

```text
======================================================
           CSMA / CD / CA CONSOLE SIMULATOR           
======================================================
1. Run Full Demo (All 3 Protocols with default params)
2. Simulate Pure CSMA (1-Persistent) [User Controlled]
3. Simulate CSMA/CD (Collision Detection) [User Controlled]
4. Simulate CSMA/CA (Collision Avoidance & RTS/CTS) [User Controlled]
5. Exit
Enter your choice [1-5] (default 1):
```

At any prompt, you can press **Enter** to accept the sensible default value or type a custom value to test different network conditions.
