# CSMA/CD & CSMA/CA Console Simulation

A standalone, single-file C++ console program demonstrating **Carrier Sense Multiple Access** (CSMA) protocols without requiring CMake, batch files, or any external dependencies.

Protocols simulated:
1. **CSMA/CD** (*Collision Detection* — IEEE 802.3 Ethernet)
2. **CSMA/CA** (*Collision Avoidance* — IEEE 802.11 Wi-Fi)

---

## Key Protocols Explained

### 1. CSMA/CD (Listen-While-Talk)
- **Carrier Sensing**: Stations check if the shared bus is idle before transmitting.
- **Collision Detection**: If two stations transmit simultaneously, both detect the collision.
- **Jam Signal & Truncated Binary Exponential Backoff (BEB)**:
  - Transmission is aborted immediately, and a jamming signal is broadcasted.
  - Stations choose a random backoff slot:
    $$R \in [0, 2^{\min(k, 10)} - 1]$$
  - Stations wait $R$ slots before re-attempting transmission.

### 2. CSMA/CA (Avoid-Before-Talk)
- **Inter-Frame Spaces (DIFS / SIFS)**: Stations sense the wireless medium idle for a Distributed Inter-Frame Space (DIFS) before initiating countdown.
- **Slotted Backoff Countdown**: Each station chooses a random backoff slot within its Contention Window ($CW$).
- **Timer Freezing**: When another station transmits, all other stations **freeze** their backoff counters and resume only after the medium is sensed idle again.
- **Acknowledgements (ACK)**: The receiver returns an ACK frame after a Short Inter-Frame Space (SIFS). If no ACK is received (collision/timeout), $CW$ doubles.

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
