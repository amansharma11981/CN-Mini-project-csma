# CSMA, CSMA/CD, and CSMA/CA Simulation in Pure C

An educational Computer Networks mini-project demonstrating **Carrier Sense Multiple Access (CSMA)** protocols in a single, clean C source file without external dependencies.

---

## Project Structure

This project contains only two files:
- `csma_simulation.c` – Complete C source code containing all three protocol simulations and an automated demo runner.
- `README.md` – Documentation, theoretical principles, and usage instructions.

---

## Protocols Simulated

### 1. Pure CSMA (1-Persistent)
- **Carrier Sensing**: A station monitors the shared medium before transmitting.
- **Rule**:
  - If the medium is **IDLE**: Transmit immediately (probability $p = 1$).
  - If the medium is **BUSY**: Continuously listen until the channel becomes IDLE, then transmit immediately ($p = 1$).
- **Vulnerability**: If multiple stations queue up while the channel is busy, they all transmit simultaneously the moment the medium frees, causing a guaranteed **collision**.

### 2. CSMA/CD (Collision Detection — IEEE 802.3 Ethernet)
- **Listen-While-Talk**: Stations continuously monitor voltage levels on the wire while transmitting.
- **Immediate Abortion & Jam Signal**: The moment overlapping signals are detected, transmission is aborted immediately to avoid wasting channel time, and a 32-bit **Jam signal** is broadcast to notify all nodes.
- **Truncated Binary Exponential Backoff (BEB)**:
  - Colliding nodes choose a random wait slot:
    $$R \in [0, 2^k - 1]$$
    where $k$ is the collision attempt number.
  - The node with the smaller backoff re-senses first, finds the channel idle, and successfully delivers its frame.

### 3. CSMA/CA (Collision Avoidance — IEEE 802.11 Wi-Fi)
- **Avoid-Before-Talk**: Wireless radios cannot easily listen while transmitting (half-duplex RF and hidden-terminal problem).
- **Inter-Frame Spaces (DIFS / SIFS)**: Stations verify the channel remains clear for a DIFS duration before beginning countdown.
- **Random Backoff**: Stations count down a randomized slot timer from the Contention Window (CW).
- **Simplified RTS/CTS Handshake**:
  1. **RTS (Request to Send)**: Sender reserves the medium with a small control frame.
  2. **CTS (Clear to Send)**: Receiver confirms reservation; hidden nodes overhear CTS and set their NAV (Network Allocation Vector) timer to remain silent.
  3. **DATA**: Sent safely without collisions.
  4. **ACK (Acknowledgment)**: Receiver confirms delivery after SIFS.

---

## Compilation

Compile with any standard C99/C11 compiler (GCC, Clang, or MSVC):

### GCC (Linux / macOS / Windows MinGW)
```bash
gcc csma_simulation.c -o csma_sim
```

### Clang
```bash
clang csma_simulation.c -o csma_sim
```

---

## Running the Simulation

### Interactive Menu
```bash
./csma_sim
```
*(On Windows PowerShell / CMD: `.\csma_sim.exe`)*

### Quick All-in-One Demo
Run the automated step-by-step demonstration of all three protocols directly:
```bash
./csma_sim --demo
```
*(On Windows: `.\csma_sim.exe --demo`)*

---

## Interactive Menu Overview

```text
======================================================
           CSMA / CD / CA CONSOLE SIMULATOR           
======================================================
1. Run Full Demo (All 3 Protocols sequentially)
2. Simulate Pure CSMA (1-Persistent)
3. Simulate CSMA/CD (Collision Detection)
4. Simulate CSMA/CA (Collision Avoidance & RTS/CTS)
5. Exit
Enter your choice (1-5):
```
