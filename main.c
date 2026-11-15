#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

// ============================================================================
// Data Structures
// ============================================================================

// Structure to represent a station on the network
typedef struct {
    int id;
    int attempts;         // Collision / retry counter
    int backoffSlots;     // Slots remaining to wait
    int cw;               // Contention Window (for CSMA/CA)
    bool transmitted;     // True if successfully sent
    bool active;          // Ready to transmit
    bool waitingBusy;     // Persistent waiting flag (carrier sensed busy)
} Station;

// Metrics structure for simulation comparison
typedef struct {
    char name[48];
    int totalStations;
    int successful;
    int dropped;
    int collisions;
    int slotsUsed;
    double successRate;
    double throughput;   // Successful packets per slot
} SimResult;

// Pure CSMA persistence strategies
typedef enum {
    CSMA_1_PERSISTENT = 1,
    CSMA_NON_PERSISTENT = 2,
    CSMA_P_PERSISTENT = 3
} CSMAMode;

// ============================================================================
// Helper Functions
// ============================================================================

static inline int clamp(int val, int minVal, int maxVal) {
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

static inline int min_int(int a, int b) {
    return (a < b) ? a : b;
}

// Read integer within [minVal, maxVal] with fallback
int getIntInput(const char* prompt, int minVal, int maxVal, int defaultVal) {
    char buf[64];
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, sizeof(buf), stdin)) {
        return defaultVal;
    }
    if (strchr(buf, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    int val;
    if (sscanf(buf, "%d", &val) != 1) {
        return defaultVal;
    }
    return clamp(val, minVal, maxVal);
}

// Read float within [minVal, maxVal] with fallback
float getFloatInput(const char* prompt, float minVal, float maxVal, float defaultVal) {
    char buf[64];
    printf("%s", prompt);
    fflush(stdout);
    if (!fgets(buf, sizeof(buf), stdin)) {
        return defaultVal;
    }
    if (strchr(buf, '\n') == NULL) {
        int c;
        while ((c = getchar()) != '\n' && c != EOF);
    }
    float val;
    if (sscanf(buf, "%f", &val) != 1) {
        return defaultVal;
    }
    if (val < minVal) return minVal;
    if (val > maxVal) return maxVal;
    return val;
}

// Find indices of active, untransmitted stations whose backoffSlots == 0
int getReadyStations(const Station* stations, int n, int* readyIndices) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (stations[i].active && !stations[i].transmitted && stations[i].backoffSlots == 0) {
            readyIndices[count++] = i;
        }
    }
    return count;
}

// Decrement remaining backoff counters for active waiting stations
void decrementBackoffs(Station* stations, int n, bool printStatus) {
    for (int i = 0; i < n; i++) {
        if (stations[i].active && !stations[i].transmitted && stations[i].backoffSlots > 0) {
            stations[i].backoffSlots--;
            if (printStatus) {
                printf("      Station %d backoff remaining: %d slots\n", stations[i].id, stations[i].backoffSlots);
            }
        }
    }
}

// Initialize station collection
void initStations(Station* stations, int n, int cwInitial, bool verbose) {
    for (int i = 0; i < n; i++) {
        stations[i].id = i + 1;
        stations[i].attempts = 0;
        stations[i].backoffSlots = (cwInitial > 0) ? (rand() % cwInitial) : 0;
        stations[i].cw = cwInitial;
        stations[i].transmitted = false;
        stations[i].active = true;
        stations[i].waitingBusy = false;

        if (verbose && cwInitial > 0) {
            printf("  Station %d -> CW: %d, Initial Backoff: %d slots\n",
                   stations[i].id, stations[i].cw, stations[i].backoffSlots);
        }
    }
}

// Print unified summary report
void printResults(const char* title, const Station* stations, int n, int collisions, int slotsUsed) {
    int successful = 0;
    for (int i = 0; i < n; i++) {
        if (stations[i].transmitted) successful++;
    }

    printf("\n============================================\n");
    printf("           %s RESULTS                  \n", title);
    printf("============================================\n");
    printf("Total Stations:          %d\n", n);
    printf("Successfully Delivered:  %d\n", successful);
    printf("Frames Dropped:          %d\n", n - successful);
    printf("Total Collisions:        %d\n", collisions);
    printf("Total Time Slots Used:   %d\n", slotsUsed);
    printf("Success Rate:            %.1f%%\n", (successful * 100.0) / n);
    printf("Throughput:              %.2f frames/slot\n", (double)successful / (slotsUsed > 0 ? slotsUsed : 1));
    printf("============================================\n");
}

// ============================================================================
// 1. CSMA/CD (Collision Detection — IEEE 802.3 Ethernet)
// ============================================================================

void runCSMACD(int n, int maxRetries, bool verbose, SimResult* res) {
    Station stations[10];
    initStations(stations, n, 0, false);

    int slot = 1, completed = 0, collisions = 0;
    int ready[10];

    if (verbose) {
        printf("\n--- Starting CSMA/CD Simulation ---\n");
    }

    while (completed < n && slot <= 50) {
        if (verbose) printf("\n[Time Slot %d]\n", slot);
        int readyCount = getReadyStations(stations, n, ready);

        if (readyCount == 0) {
            if (verbose) printf("  Channel Status: IDLE (Stations waiting in backoff)\n");
        } else if (readyCount == 1) {
            int idx = ready[0];
            stations[idx].transmitted = true;
            stations[idx].active = false;
            completed++;
            if (verbose) {
                printf("  Channel Status: SUCCESS\n");
                printf("  --> Station %d successfully transmitted its frame!\n", stations[idx].id);
            }
        } else {
            collisions++;
            if (verbose) {
                printf("  Channel Status: COLLISION DETECTED!\n  --> Simultaneous transmission by: ");
                for (int i = 0; i < readyCount; i++) {
                    printf("Station %d ", stations[ready[i]].id);
                }
                printf("\n  --> Jamming signal broadcasted; transmission aborted.\n");
            }

            for (int i = 0; i < readyCount; i++) {
                int idx = ready[i];
                stations[idx].attempts++;
                int k = stations[idx].attempts;

                if (k > maxRetries) {
                    if (verbose) {
                        printf("  [!] Station %d reached max retries (%d). Frame DROPPED.\n",
                               stations[idx].id, maxRetries);
                    }
                    stations[idx].active = false;
                    completed++;
                } else {
                    int exponent = min_int(k, 10);
                    int maxR = (1 << exponent) - 1;
                    int r = rand() % (maxR + 1);
                    stations[idx].backoffSlots = r;

                    if (verbose) {
                        printf("      Station %d (Attempt %d): Backoff R = %d slots (range [0, %d])\n",
                               stations[idx].id, k, r, maxR);
                    }
                }
            }
        }

        decrementBackoffs(stations, n, false);
        slot++;
    }

    int usedSlots = (slot > 1) ? (slot - 1) : 1;
    int successful = 0;
    for (int i = 0; i < n; i++) {
        if (stations[i].transmitted) successful++;
    }

    if (verbose) {
        printResults("CSMA/CD", stations, n, collisions, usedSlots);
    }

    if (res) {
        snprintf(res->name, sizeof(res->name), "CSMA/CD (IEEE 802.3)");
        res->totalStations = n;
        res->successful = successful;
        res->dropped = n - successful;
        res->collisions = collisions;
        res->slotsUsed = usedSlots;
        res->successRate = (successful * 100.0) / n;
        res->throughput = (double)successful / usedSlots;
    }
}

void simulateCSMACD(void) {
    printf("\n============================================\n");
    printf("     CSMA/CD SIMULATION (IEEE 802.3)        \n");
    printf("  Principle: Listen While Talk + Jam + BEB  \n");
    printf("============================================\n");

    int n = getIntInput("Enter number of stations (2 to 10, default 4): ", 2, 10, 4);
    int maxRetries = getIntInput("Enter max collision attempts (default 10 or 16): ", 1, 16, 10);

    runCSMACD(n, maxRetries, true, NULL);
}

// ============================================================================
// 2. CSMA/CA (Collision Avoidance — IEEE 802.11 Wi-Fi)
// ============================================================================

void runCSMACA(int n, bool verbose, SimResult* res) {
    Station stations[10];
    initStations(stations, n, 8, verbose); // cwMin = 8

    int slot = 1, completed = 0, collisions = 0;
    int ready[10];

    if (verbose) {
        printf("\n--- Starting CSMA/CA Slotted Contention Simulation ---\n");
    }

    while (completed < n && slot <= 50) {
        if (verbose) printf("\n[Time Slot %d]\n", slot);
        int readyCount = getReadyStations(stations, n, ready);

        if (readyCount == 0) {
            if (verbose) {
                printf("  Medium Status: IDLE (DIFS sensed clear)\n");
                printf("  --> Active stations decrementing backoff counter:\n");
            }
            decrementBackoffs(stations, n, verbose);
        } else if (readyCount == 1) {
            int idx = ready[0];
            stations[idx].transmitted = true;
            stations[idx].active = false;
            completed++;

            if (verbose) {
                printf("  Medium Status: BUSY (Station %d Transmitting)\n", stations[idx].id);
                printf("  --> Frame transmitted successfully over wireless medium.\n");
                printf("  --> SIFS interval observed; Receiver returned ACK frame.\n");
                printf("  --> Other stations sense carrier and FREEZE backoff counters:\n");
                for (int i = 0; i < n; i++) {
                    if (stations[i].active && !stations[i].transmitted) {
                        printf("      Station %d frozen at %d slots\n", stations[i].id, stations[i].backoffSlots);
                    }
                }
            }
        } else {
            collisions++;
            if (verbose) {
                printf("  Medium Status: COLLISION (Multiple stations reached 0)\n  --> Colliding Stations: ");
                for (int i = 0; i < readyCount; i++) {
                    printf("Station %d ", stations[ready[i]].id);
                }
                printf("\n  --> No ACK received (Timeout). Doubling Contention Window (CW):\n");
            }

            for (int i = 0; i < readyCount; i++) {
                int idx = ready[i];
                stations[idx].attempts++;
                stations[idx].cw = min_int(stations[idx].cw * 2, 64);
                stations[idx].backoffSlots = rand() % stations[idx].cw;

                if (verbose) {
                    printf("      Station %d: New CW = %d, New Backoff = %d slots\n",
                           stations[idx].id, stations[idx].cw, stations[idx].backoffSlots);
                }
            }
        }

        slot++;
    }

    int usedSlots = (slot > 1) ? (slot - 1) : 1;
    int successful = 0;
    for (int i = 0; i < n; i++) {
        if (stations[i].transmitted) successful++;
    }

    if (verbose) {
        printResults("CSMA/CA", stations, n, collisions, usedSlots);
    }

    if (res) {
        snprintf(res->name, sizeof(res->name), "CSMA/CA (IEEE 802.11)");
        res->totalStations = n;
        res->successful = successful;
        res->dropped = n - successful;
        res->collisions = collisions;
        res->slotsUsed = usedSlots;
        res->successRate = (successful * 100.0) / n;
        res->throughput = (double)successful / usedSlots;
    }
}

void simulateCSMACA(void) {
    printf("\n============================================\n");
    printf("     CSMA/CA SIMULATION (IEEE 802.11)       \n");
    printf("  Principle: Avoid Before Talk + DIFS/ACK   \n");
    printf("============================================\n");

    int n = getIntInput("Enter number of wireless stations (2 to 8, default 3): ", 2, 8, 3);
    runCSMACA(n, true, NULL);
}

// ============================================================================
// 3. Pure CSMA (1-Persistent, Non-Persistent, p-Persistent)
// ============================================================================

void runCSMA(CSMAMode mode, float p_val, int n, int frameDuration, bool verbose, SimResult* res) {
    const char* modeName = (mode == CSMA_1_PERSISTENT) ? "1-Persistent CSMA" :
                           (mode == CSMA_NON_PERSISTENT) ? "Non-Persistent CSMA" :
                           "p-Persistent CSMA";

    Station stations[10];
    initStations(stations, n, 0, false);

    int slot = 1;
    int completed = 0;
    int collisions = 0;
    int maxRetries = 10;
    int maxSlots = 50;

    int currentTransmitter = -1;
    int busyUntilSlot = 0;
    bool currentIsCollision = false;

    int collidingStations[10];
    int collidingCount = 0;

    if (verbose) {
        printf("\n--- Starting %s Simulation (Frame Duration: %d slots) ---\n", modeName, frameDuration);
        if (mode == CSMA_P_PERSISTENT) {
            printf("  Parameter p = %.2f (probability of transmitting on idle slot)\n", p_val);
        }
    }

    while (completed < n && slot <= maxSlots) {
        if (verbose) {
            printf("\n[Time Slot %d]\n", slot);
        }

        // 1. Check if previous transmission ended before this slot
        if (busyUntilSlot < slot && currentTransmitter != -1) {
            if (currentIsCollision) {
                if (verbose) {
                    printf("  Channel Event: Previous colliding frames finished transmission.\n");
                    printf("  --> No ACK received. Colliding stations enter random backoff:\n");
                }
                for (int i = 0; i < collidingCount; i++) {
                    int idx = collidingStations[i];
                    stations[idx].attempts++;
                    int k = stations[idx].attempts;
                    if (k > maxRetries) {
                        if (verbose) {
                            printf("      [!] Station %d exceeded max retries (%d). Frame DROPPED.\n",
                                   stations[idx].id, maxRetries);
                        }
                        stations[idx].active = false;
                        completed++;
                    } else {
                        int maxR = (mode == CSMA_NON_PERSISTENT) ? 5 : ((1 << min_int(k, 4)) + 1);
                        int r = 1 + (rand() % maxR);
                        stations[idx].backoffSlots = r;
                        if (verbose) {
                            printf("      Station %d (Attempt %d): Backoff R = %d slots\n",
                                   stations[idx].id, k, r);
                        }
                    }
                }
                collidingCount = 0;
                currentIsCollision = false;
            } else {
                stations[currentTransmitter].transmitted = true;
                stations[currentTransmitter].active = false;
                completed++;
                if (verbose) {
                    printf("  Channel Event: Station %d transmission COMPLETED successfully (ACK received)!\n",
                           stations[currentTransmitter].id);
                }
            }
            currentTransmitter = -1;
        }

        bool channelBusy = (busyUntilSlot >= slot && currentTransmitter != -1);

        // 2. Identify candidate stations ready to sense the channel
        int candidates[10];
        int candidateCount = 0;

        for (int i = 0; i < n; i++) {
            if (!stations[i].active || stations[i].transmitted) continue;
            if (i == currentTransmitter) continue;

            bool currentlySending = false;
            for (int c = 0; c < collidingCount; c++) {
                if (collidingStations[c] == i) {
                    currentlySending = true;
                    break;
                }
            }
            if (currentlySending) continue;

            if (stations[i].backoffSlots == 0 || stations[i].waitingBusy) {
                candidates[candidateCount++] = i;
            }
        }

        // 3. Process candidate stations based on channel state
        int willTransmit[10];
        int txCount = 0;

        if (channelBusy) {
            if (verbose) {
                if (currentIsCollision) {
                    printf("  Channel Status: BUSY (Colliding frames occupying medium until slot %d)\n", busyUntilSlot);
                } else {
                    printf("  Channel Status: BUSY (Station %d transmitting until slot %d)\n",
                           stations[currentTransmitter].id, busyUntilSlot);
                }
            }

            for (int i = 0; i < candidateCount; i++) {
                int idx = candidates[i];
                if (mode == CSMA_1_PERSISTENT) {
                    stations[idx].waitingBusy = true;
                    if (verbose) {
                        printf("  --> Station %d senses BUSY: Persistently waiting for channel to clear.\n",
                               stations[idx].id);
                    }
                } else if (mode == CSMA_NON_PERSISTENT) {
                    stations[idx].waitingBusy = false;
                    int r = 1 + (rand() % 5);
                    stations[idx].backoffSlots = r;
                    if (verbose) {
                        printf("  --> Station %d senses BUSY: Non-persistent backoff R = %d slots before re-sensing.\n",
                               stations[idx].id, r);
                    }
                } else if (mode == CSMA_P_PERSISTENT) {
                    stations[idx].waitingBusy = true;
                    if (verbose) {
                        printf("  --> Station %d senses BUSY: Waiting until channel becomes IDLE.\n",
                               stations[idx].id);
                    }
                }
            }
        } else {
            // Channel is IDLE
            if (verbose) {
                printf("  Channel Status: IDLE\n");
            }

            for (int i = 0; i < candidateCount; i++) {
                int idx = candidates[i];
                if (mode == CSMA_1_PERSISTENT || mode == CSMA_NON_PERSISTENT) {
                    stations[idx].waitingBusy = false;
                    willTransmit[txCount++] = idx;
                } else if (mode == CSMA_P_PERSISTENT) {
                    stations[idx].waitingBusy = false;
                    float roll = (float)rand() / (float)RAND_MAX;
                    if (roll < p_val) {
                        willTransmit[txCount++] = idx;
                        if (verbose) {
                            printf("  --> Station %d rolled %.2f (< %.2f): Decides to TRANSMIT!\n",
                                   stations[idx].id, roll, p_val);
                        }
                    } else {
                        stations[idx].backoffSlots = 1; // Defer by 1 slot
                        if (verbose) {
                            printf("  --> Station %d rolled %.2f (>= %.2f): DEFERS to next slot.\n",
                                   stations[idx].id, roll, p_val);
                        }
                    }
                }
            }

            // Execute transmissions
            if (txCount == 0) {
                if (verbose && candidateCount > 0) {
                    printf("  --> All candidates deferred; channel stays IDLE.\n");
                }
            } else if (txCount == 1) {
                int idx = willTransmit[0];
                currentTransmitter = idx;
                currentIsCollision = false;
                busyUntilSlot = slot + frameDuration - 1;

                if (verbose) {
                    printf("  --> SUCCESS: Station %d starts transmission (duration: %d slot%s, ends at slot %d).\n",
                           stations[idx].id, frameDuration, (frameDuration > 1 ? "s" : ""), busyUntilSlot);
                }

                if (frameDuration == 1) {
                    stations[idx].transmitted = true;
                    stations[idx].active = false;
                    completed++;
                    currentTransmitter = -1;
                    busyUntilSlot = 0;
                    if (verbose) {
                        printf("  --> Station %d frame delivered immediately!\n", stations[idx].id);
                    }
                }
            } else {
                // Collision!
                collisions++;
                currentIsCollision = true;
                currentTransmitter = willTransmit[0];
                busyUntilSlot = slot + frameDuration - 1;
                collidingCount = txCount;
                for (int i = 0; i < txCount; i++) {
                    collidingStations[i] = willTransmit[i];
                }

                if (verbose) {
                    printf("  --> COLLISION DETECTED! Stations transmitting simultaneously: ");
                    for (int i = 0; i < txCount; i++) {
                        printf("Station %d ", stations[willTransmit[i]].id);
                    }
                    printf("\n  --> In pure CSMA, colliding frames occupy medium for %d slot%s.\n",
                           frameDuration, (frameDuration > 1 ? "s" : ""));
                }

                if (frameDuration == 1) {
                    for (int i = 0; i < collidingCount; i++) {
                        int idx = collidingStations[i];
                        stations[idx].attempts++;
                        int k = stations[idx].attempts;
                        if (k > maxRetries) {
                            stations[idx].active = false;
                            completed++;
                            if (verbose) printf("      Station %d frame DROPPED.\n", stations[idx].id);
                        } else {
                            int maxR = (mode == CSMA_NON_PERSISTENT) ? 5 : ((1 << min_int(k, 4)) + 1);
                            stations[idx].backoffSlots = 1 + (rand() % maxR);
                            if (verbose) printf("      Station %d backoff R = %d slots\n", stations[idx].id, stations[idx].backoffSlots);
                        }
                    }
                    collidingCount = 0;
                    currentIsCollision = false;
                    currentTransmitter = -1;
                    busyUntilSlot = 0;
                }
            }
        }

        decrementBackoffs(stations, n, false);
        slot++;
    }

    // Wrap up any ongoing successful transmission if slot limit reached
    if (currentTransmitter != -1 && !currentIsCollision && busyUntilSlot <= slot) {
        stations[currentTransmitter].transmitted = true;
        stations[currentTransmitter].active = false;
        completed++;
    }

    int successful = 0;
    for (int i = 0; i < n; i++) {
        if (stations[i].transmitted) successful++;
    }

    int usedSlots = (slot > 1) ? (slot - 1) : 1;

    if (verbose) {
        printResults(modeName, stations, n, collisions, usedSlots);
    }

    if (res) {
        if (mode == CSMA_P_PERSISTENT) {
            snprintf(res->name, sizeof(res->name), "CSMA (p=%.1f-Persistent)", p_val);
        } else {
            snprintf(res->name, sizeof(res->name), "%s", modeName);
        }
        res->totalStations = n;
        res->successful = successful;
        res->dropped = n - successful;
        res->collisions = collisions;
        res->slotsUsed = usedSlots;
        res->successRate = (successful * 100.0) / n;
        res->throughput = (double)successful / usedSlots;
    }
}

void comparePersistenceModes(int n, int frameDuration) {
    printf("\n======================================================================\n");
    printf("               CSMA PERSISTENCE COMPARISON BENCHMARK                  \n");
    printf("                    (Stations: %d, Frame: %d slots)                    \n", n, frameDuration);
    printf("======================================================================\n");

    SimResult results[4];
    runCSMA(CSMA_1_PERSISTENT, 1.0f, n, frameDuration, false, &results[0]);
    runCSMA(CSMA_NON_PERSISTENT, 0.0f, n, frameDuration, false, &results[1]);
    runCSMA(CSMA_P_PERSISTENT, 0.5f, n, frameDuration, false, &results[2]);
    runCSMA(CSMA_P_PERSISTENT, 0.2f, n, frameDuration, false, &results[3]);

    printf("%-26s | %-9s | %-7s | %-10s | %-5s | %-10s\n",
           "Strategy", "Delivered", "Dropped", "Collisions", "Slots", "Throughput");
    printf("---------------------------+-----------+---------+------------+-------+-----------\n");

    for (int i = 0; i < 4; i++) {
        printf("%-26s | %4d / %-2d | %-7d | %-10d | %-5d | %5.2f p/s\n",
               results[i].name,
               results[i].successful, results[i].totalStations,
               results[i].dropped,
               results[i].collisions,
               results[i].slotsUsed,
               results[i].throughput);
    }
    printf("======================================================================\n");
    printf("Key Insights:\n");
    printf(" - 1-Persistent: High collision risk; stations waiting during busy periods all transmit at once.\n");
    printf(" - Non-Persistent: Drastically cuts collisions by randomizing re-sensing times; higher idle delay.\n");
    printf(" - p-Persistent: Tunable parameter p balances channel utilization and collision probability.\n");
    printf("======================================================================\n");
}

void simulatePureCSMA(void) {
    while (1) {
        printf("\n============================================\n");
        printf("        PURE CSMA PERSISTENCE MODES         \n");
        printf("  Carrier Sense Multiple Access Simulation  \n");
        printf("============================================\n");
        printf("1. 1-Persistent CSMA     (Continuous sense, p=1 on idle)\n");
        printf("2. Non-Persistent CSMA   (Random backoff when busy)\n");
        printf("3. p-Persistent CSMA     (Slotted: transmit with prob p)\n");
        printf("4. Compare All 3 Persistence Modes Side-by-Side\n");
        printf("5. Return to Main Menu\n");

        int modeChoice = getIntInput("Enter choice (1-5): ", 1, 5, 1);
        if (modeChoice == 5) {
            break;
        }

        int n = getIntInput("Enter number of stations (2 to 10, default 4): ", 2, 10, 4);
        int frameDuration = getIntInput("Enter frame duration in slots (1 to 4, default 2): ", 1, 4, 2);

        if (modeChoice == 1) {
            runCSMA(CSMA_1_PERSISTENT, 1.0f, n, frameDuration, true, NULL);
        } else if (modeChoice == 2) {
            runCSMA(CSMA_NON_PERSISTENT, 0.0f, n, frameDuration, true, NULL);
        } else if (modeChoice == 3) {
            float p_val = getFloatInput("Enter transmission probability p (0.1 to 1.0, default 0.5): ", 0.1f, 1.0f, 0.5f);
            runCSMA(CSMA_P_PERSISTENT, p_val, n, frameDuration, true, NULL);
        } else if (modeChoice == 4) {
            comparePersistenceModes(n, frameDuration);
        }
    }
}

// ============================================================================
// 4. Comprehensive Protocol Benchmark (CSMA vs CSMA/CD vs CSMA/CA)
// ============================================================================

void compareAllProtocols(void) {
    int n = getIntInput("Enter number of stations to benchmark (2 to 8, default 4): ", 2, 8, 4);

    printf("\n========================================================================================\n");
    printf("                       COMPREHENSIVE PROTOCOL COMPARISON BENCHMARK                      \n");
    printf("                             (Simulating N = %d Stations)                                \n", n);
    printf("========================================================================================\n");

    SimResult results[5];
    runCSMA(CSMA_1_PERSISTENT, 1.0f, n, 2, false, &results[0]);
    runCSMA(CSMA_NON_PERSISTENT, 0.0f, n, 2, false, &results[1]);
    runCSMA(CSMA_P_PERSISTENT, 0.5f, n, 2, false, &results[2]);
    runCSMACD(n, 10, false, &results[3]);
    runCSMACA(n, false, &results[4]);

    printf("%-28s | %-9s | %-7s | %-10s | %-5s | %-8s | %-10s\n",
           "Protocol / Mechanism", "Delivered", "Dropped", "Collisions", "Slots", "Success%", "Throughput");
    printf("-----------------------------+-----------+---------+------------+-------+----------+-----------\n");

    for (int i = 0; i < 5; i++) {
        printf("%-28s | %4d / %-2d | %-7d | %-10d | %-5d | %6.1f%% | %5.2f p/s\n",
               results[i].name,
               results[i].successful, results[i].totalStations,
               results[i].dropped,
               results[i].collisions,
               results[i].slotsUsed,
               results[i].successRate,
               results[i].throughput);
    }

    printf("========================================================================================\n");
    printf("Summary of Protocol Characteristics:\n");
    printf(" * 1-Persistent CSMA   : Greedy; senses continuously and transmits immediately when free.\n");
    printf(" * Non-Persistent CSMA : Senses channel; if busy, backs off without queueing at medium.\n");
    printf(" * p-Persistent CSMA   : Slotted; transmits with probability p when idle; avoids clumping.\n");
    printf(" * CSMA/CD             : Ethernet; detects collisions in progress and jams immediately.\n");
    printf(" * CSMA/CA             : Wi-Fi; avoids collisions via DIFS/SIFS intervals & frozen backoffs.\n");
    printf("========================================================================================\n");
}

// ============================================================================
// Main Interactive Menu
// ============================================================================

int main(void) {
    srand((unsigned int)time(NULL));

    while (1) {
        printf("\n====================================\n");
        printf("       CSMA CONSOLE SIMULATOR       \n");
        printf("====================================\n");
        printf("1. Simulate CSMA/CD (Collision Detection -- IEEE 802.3 Ethernet)\n");
        printf("2. Simulate CSMA/CA (Collision Avoidance -- IEEE 802.11 Wi-Fi)\n");
        printf("3. Simulate Pure CSMA (1-Persistent, Non-Persistent, p-Persistent)\n");
        printf("4. Compare All Protocols Benchmark (CSMA vs CSMA/CD vs CSMA/CA)\n");
        printf("5. Exit\n");

        int choice = getIntInput("Enter your choice (1-5): ", 1, 5, 0);

        if (choice == 1) {
            simulateCSMACD();
        } else if (choice == 2) {
            simulateCSMACA();
        } else if (choice == 3) {
            simulatePureCSMA();
        } else if (choice == 4) {
            compareAllProtocols();
        } else if (choice == 5) {
            printf("\nExiting CSMA Simulator. Thank you!\n");
            break;
        } else {
            printf("Invalid choice! Please enter a number between 1 and 5.\n");
        }
    }

    return 0;
}
