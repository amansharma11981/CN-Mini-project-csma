# CSMA, CSMA/CD & CSMA/CA Console Simulation

A standalone, single-file C console program demonstrating **Carrier Sense Multiple Access (CSMA)** protocols without requiring CMake, batch files, or external libraries.

Designed for Computer Networks (CN) education and laboratory experiments.

---

## Protocols Implemented

1. **Pure CSMA (Persistence Modes)**
   - **1-Persistent CSMA**: Continuous sensing; transmits with probability $p = 1$ the moment the channel becomes idle.
   - **Non-Persistent CSMA**: Senses channel; if busy, waits a random backoff time before sensing again (does not queue at the channel).
   - **p-Persistent CSMA**: Slotted access; when the channel is idle, transmits with probability $p$ and defers to the next slot with probability $1 - p$.
2. **CSMA/CD** (*Collision Detection* — IEEE 802.3 Ethernet)
   - Listen-while-talk mechanism, instant transmission abortion, Jam signal broadcasting, and Truncated Binary Exponential Backoff (BEB).
3. **CSMA/CA** (*Collision Avoidance* — IEEE 802.11 Wi-Fi)
   - Avoid-before-talk mechanism, DIFS sensing, Slotted Contention Window countdown, backoff timer freezing during foreign transmissions, and SIFS + ACK verification.
4. **Interactive Benchmarking & Comparison**
   - Side-by-side performance comparisons across all persistence strategies and media access protocols under identical traffic loads.

---

## Key Protocols Explained

### 1. Pure CSMA Persistence Strategies

| Strategy | When Channel is Idle | When Channel is Busy | Collision Probability | Channel Utilization |
| :--- | :--- | :--- | :--- | :--- |
| **1-Persistent** | Transmit immediately ($p = 1$) | Continuously monitor until idle, then transmit immediately | **High** (waiting stations collide when channel frees) | High under light traffic |
| **Non-Persistent** | Transmit immediately | Do not monitor; back off for a random interval, then re-sense | **Low** (stations desynchronize their sensing times) | Lower under light traffic (idle gaps) |
| **p-Persistent** | Transmit with probability $p$; defer 1 slot with probability $1 - p$ | Wait until idle, then apply $p$-probability trial | **Tunable** (balances collision vs. delay) | Optimized for slotted networks |

### 2. CSMA/CD (Listen-While-Talk)
- **Carrier Sensing**: Stations check if the shared bus is idle before transmitting.
- **Collision Detection**: If multiple stations transmit concurrently, both detect the collision while transmitting.
- **Jam Signal & Truncated Binary Exponential Backoff (BEB)**:
  - Transmission is aborted immediately, and a 32/48-bit jamming signal is broadcasted.
  - Stations choose a random backoff slot:
    $$R \in [0, 2^{\min(k, 10)} - 1]$$
  - Stations wait $R$ slots before re-attempting transmission.

### 3. CSMA/CA (Avoid-Before-Talk)
- **Inter-Frame Spaces (DIFS / SIFS)**: Wireless stations sense the medium idle for a Distributed Inter-Frame Space (DIFS) before initiating countdown.
- **Slotted Backoff Countdown**: Each station chooses a random backoff slot within its Contention Window ($CW$).
- **Timer Freezing**: When another station transmits, all other stations **freeze** their backoff counters and resume only after the medium is sensed idle again.
- **Acknowledgements (ACK)**: The receiver returns an ACK frame after a Short Inter-Frame Space (SIFS). If no ACK is received (collision/timeout), $CW$ doubles ($CW_{\max} = 64$).

---

## Comprehensive Benchmark Output Example

```text
========================================================================================
                       COMPREHENSIVE PROTOCOL COMPARISON BENCHMARK                      
                             (Simulating N = 4 Stations)                                
========================================================================================
Protocol / Mechanism         | Delivered | Dropped | Collisions | Slots | Success% | Throughput
-----------------------------+-----------+---------+------------+-------+----------+-----------
1-Persistent CSMA            |    4 / 4  | 0       | 3          | 25    |  100.0% |  0.16 p/s
Non-Persistent CSMA          |    4 / 4  | 0       | 2          | 15    |  100.0% |  0.27 p/s
CSMA (p=0.5-Persistent)      |    4 / 4  | 0       | 2          | 20    |  100.0% |  0.20 p/s
CSMA/CD (IEEE 802.3)         |    4 / 4  | 0       | 3          | 10    |  100.0% |  0.40 p/s
CSMA/CA (IEEE 802.11)        |    4 / 4  | 0       | 1          | 21    |  100.0% |  0.19 p/s
========================================================================================
```

---

## How to Compile & Run

This project consists of only `main.c` using standard C99 (`stdio.h`, `stdlib.h`, `time.h`, `stdbool.h`, `string.h`). You can compile it directly with any standard C compiler:

### GCC / MinGW (Linux / Windows)
```bash
gcc -Wall -Wextra -std=c99 main.c -o csma_sim
./csma_sim
```
*(On Windows Command Prompt / PowerShell: `gcc -Wall -Wextra -std=c99 main.c -o csma_sim.exe` followed by `.\csma_sim.exe`)*

### Clang
```bash
clang -Wall -Wextra -std=c99 main.c -o csma_sim
./csma_sim
```

### MSVC (Visual Studio Developer Command Prompt)
```cmd
cl /W4 main.c
main.exe
```
