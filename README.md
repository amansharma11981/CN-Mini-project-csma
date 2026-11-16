# CSMA, CSMA/CD & CSMA/CA Console Simulation

A clean, standalone, single-file C++ console project demonstrating **Carrier Sense Multiple Access** (CSMA) protocols without external dependencies or build tools:
1. **Regular CSMA** (*Carrier Sense Multiple Access* — 1-Persistent, Non-Persistent, $p$-Persistent)
2. **CSMA/CD** (*Collision Detection* — IEEE 802.3 Ethernet)
3. **CSMA/CA** (*Collision Avoidance* — IEEE 802.11 Wi-Fi)

---

## Protocols Explained

### 1. Regular CSMA (Listen-Before-Talk)
- **Carrier Sensing**: Stations check if the shared medium is idle before transmitting.
- **Persistence Strategies**:
  - **1-Persistent**: Senses channel; if idle, transmits immediately ($p=1$). If busy, listens continuously until idle, then transmits.
  - **Non-Persistent**: Senses channel; if idle, transmits immediately. If busy, waits a random backoff time before sensing again to reduce collisions.
  - **$p$-Persistent**: Used in slotted channels; if idle, transmits with probability $p$, or defers to the next slot with probability $1-p$.
- **No Collision Detection during transmission**: Colliding transmissions occupy the medium for the full duration before collision is recognized (timeout).

### 2. CSMA/CD (Listen-While-Talk)
- **Carrier Sensing**: Stations check if the shared bus is idle before transmitting.
- **Collision Detection**: If two stations transmit simultaneously, both detect the collision on the wire.
- **Jam Signal & Truncated Binary Exponential Backoff (BEB)**:
  - Transmission is aborted immediately, and a 32-bit jamming signal is broadcasted.
  - Stations choose a random backoff slot:
    $$R \in [0, 2^{\min(k, 10)} - 1]$$
  - Stations wait $R$ slots before re-attempting transmission.

### 3. CSMA/CA (Avoid-Before-Talk)
- **Inter-Frame Spaces (DIFS / SIFS)**: Stations must sense the wireless medium idle for DIFS before initiating countdown.
- **Slotted Backoff Countdown**: Each station chooses a random backoff within its Contention Window ($CW$).
- **Timer Freezing**: When another station transmits, all other stations **freeze** their backoff countdown and resume only after the medium clears.
- **Acknowledgements (ACK)**: Receiver returns an ACK frame after a SIFS duration. If no ACK is received (collision/timeout), $CW$ doubles.

---

## How to Compile & Run

This project consists of only `main.cpp` using the C++ Standard Library. You can compile it directly with any standard C++ compiler:

### GCC / MinGW (Windows / Linux)
```bash
g++ main.cpp -o csma_sim
./csma_sim
```
*(On Windows Command Prompt: `g++ main.cpp -o csma_sim.exe` followed by `csma_sim.exe`)*

### Clang
```bash
clang++ main.cpp -o csma_sim
./csma_sim
```

### MSVC (Visual Studio Developer Command Prompt)
```cmd
cl /EHsc main.cpp
main.exe
```
