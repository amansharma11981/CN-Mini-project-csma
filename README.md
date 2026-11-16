# CSMA/CD & CSMA/CA Console Simulation

A simple, readable single-file C++ console project demonstrating **Carrier Sense Multiple Access** protocols:
1. **CSMA/CD** (*Collision Detection* — IEEE 802.3 Ethernet)
2. **CSMA/CA** (*Collision Avoidance* — IEEE 802.11 Wi-Fi)
3. **CSMA Persistence Methods** (1-Persistent, Non-Persistent, p-Persistent)

---

## Key Protocols Explained

### 1. CSMA/CD (Listen-While-Talk)
- **Carrier Sensing**: Stations check if the shared bus is idle before transmitting.
- **Collision Detection**: If two stations transmit simultaneously, both detect the collision.
- **Jam Signal & Truncated Binary Exponential Backoff (BEB)**:
  - Transmission is aborted immediately, and a jamming signal is emitted.
  - Stations choose a random backoff:
    $$R \in [0, 2^{\min(k, 10)} - 1]$$
  - Stations wait $R$ slots before re-attempting transmission.

### 2. CSMA/CA (Avoid-Before-Talk)
- **Inter-Frame Spaces (DIFS / SIFS)**: Stations must sense the wireless medium idle for DIFS before countdown.
- **Slotted Backoff Countdown**: Each station chooses a random backoff within Contention Window ($CW$).
- **Timer Freezing**: When another station is transmitting, all other stations **freeze** their backoff countdown and resume only after the medium clears.
- **Acknowledgements (ACK)**: Receiver returns an ACK frame after a SIFS duration. If no ACK is received (collision), $CW$ doubles.

---

## How to Compile & Run

### Using MinGW / GCC
```cmd
g++ main.cpp -o csma_sim.exe
csma_sim.exe
```

Or run:
```cmd
build.bat
```
