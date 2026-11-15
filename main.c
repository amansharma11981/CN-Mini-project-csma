#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// Structure to represent a station on the network
typedef struct {
    int id;
    int attempts;         // Collision / retry counter
    int backoffSlots;     // Slots remaining to wait
    int cw;               // Contention Window (for CSMA/CA)
    bool transmitted;     // True if successfully sent
    bool active;          // Ready to transmit
} Station;

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

// Clear remaining characters from stdin line
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// Read integer within [minVal, maxVal] with fallback
int getIntInput(const char* prompt, int minVal, int maxVal, int defaultVal) {
    int val;
    printf("%s", prompt);
    if (scanf("%d", &val) != 1) {
        clearInputBuffer();
        return defaultVal;
    }
    clearInputBuffer();
    return clamp(val, minVal, maxVal);
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
void initStations(Station* stations, int n, int cwInitial) {
    for (int i = 0; i < n; i++) {
        stations[i].id = i + 1;
        stations[i].attempts = 0;
        stations[i].backoffSlots = (cwInitial > 0) ? (rand() % cwInitial) : 0;
        stations[i].cw = cwInitial;
        stations[i].transmitted = false;
        stations[i].active = true;

        if (cwInitial > 0) {
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
    printf("============================================\n");
}

// ============================================================================
// 1. CSMA/CD (Collision Detection — IEEE 802.3 Ethernet)
// ============================================================================
void simulateCSMACD(void) {
    printf("\n============================================\n");
    printf("     CSMA/CD SIMULATION (IEEE 802.3)        \n");
    printf("  Principle: Listen While Talk + Jam + BEB  \n");
    printf("============================================\n");

    int n = getIntInput("Enter number of stations (2 to 10): ", 2, 10, 4);
    int maxRetries = getIntInput("Enter max collision attempts (default 10 or 16): ", 1, 16, 10);

    Station stations[10];
    initStations(stations, n, 0);

    int slot = 1, completed = 0, collisions = 0;
    int ready[10];

    printf("\n--- Starting CSMA/CD Simulation ---\n");

    while (completed < n && slot <= 50) {
        printf("\n[Time Slot %d]\n", slot);
        int readyCount = getReadyStations(stations, n, ready);

        if (readyCount == 0) {
            printf("  Channel Status: IDLE (Stations waiting in backoff)\n");
        } else if (readyCount == 1) {
            int idx = ready[0];
            stations[idx].transmitted = true;
            stations[idx].active = false;
            completed++;
            printf("  Channel Status: SUCCESS\n");
            printf("  --> Station %d successfully transmitted its frame!\n", stations[idx].id);
        } else {
            collisions++;
            printf("  Channel Status: COLLISION DETECTED!\n  --> Simultaneous transmission by: ");
            for (int i = 0; i < readyCount; i++) {
                printf("Station %d ", stations[ready[i]].id);
            }
            printf("\n  --> Jamming signal broadcasted; transmission aborted.\n");

            for (int i = 0; i < readyCount; i++) {
                int idx = ready[i];
                stations[idx].attempts++;
                int k = stations[idx].attempts;

                if (k > maxRetries) {
                    printf("  [!] Station %d reached max retries (%d). Frame DROPPED.\n",
                           stations[idx].id, maxRetries);
                    stations[idx].active = false;
                    completed++;
                } else {
                    int exponent = min_int(k, 10);
                    int maxR = (1 << exponent) - 1;
                    int r = rand() % (maxR + 1);
                    stations[idx].backoffSlots = r;

                    printf("      Station %d (Attempt %d): Backoff R = %d slots (range [0, %d])\n",
                           stations[idx].id, k, r, maxR);
                }
            }
        }

        decrementBackoffs(stations, n, false);
        slot++;
    }

    printResults("CSMA/CD", stations, n, collisions, slot - 1);
}

// ============================================================================
// 2. CSMA/CA (Collision Avoidance — IEEE 802.11 Wi-Fi)
// ============================================================================
void simulateCSMACA(void) {
    printf("\n============================================\n");
    printf("     CSMA/CA SIMULATION (IEEE 802.11)       \n");
    printf("  Principle: Avoid Before Talk + DIFS/ACK   \n");
    printf("============================================\n");

    int n = getIntInput("Enter number of wireless stations (2 to 8): ", 2, 8, 3);

    printf("\nInitializing Station Contention Windows & Backoff Timers:\n");
    Station stations[8];
    initStations(stations, n, 8); // cwMin = 8

    int slot = 1, completed = 0, collisions = 0;
    int ready[8];

    printf("\n--- Starting CSMA/CA Slotted Contention Simulation ---\n");

    while (completed < n && slot <= 50) {
        printf("\n[Time Slot %d]\n", slot);
        int readyCount = getReadyStations(stations, n, ready);

        if (readyCount == 0) {
            printf("  Medium Status: IDLE (DIFS sensed clear)\n");
            printf("  --> Active stations decrementing backoff counter:\n");
            decrementBackoffs(stations, n, true);
        } else if (readyCount == 1) {
            int idx = ready[0];
            stations[idx].transmitted = true;
            stations[idx].active = false;
            completed++;

            printf("  Medium Status: BUSY (Station %d Transmitting)\n", stations[idx].id);
            printf("  --> Frame transmitted successfully over wireless medium.\n");
            printf("  --> SIFS interval observed; Receiver returned ACK frame.\n");
            printf("  --> Other stations sense carrier and FREEZE backoff counters:\n");

            for (int i = 0; i < n; i++) {
                if (stations[i].active && !stations[i].transmitted) {
                    printf("      Station %d frozen at %d slots\n", stations[i].id, stations[i].backoffSlots);
                }
            }
        } else {
            collisions++;
            printf("  Medium Status: COLLISION (Multiple stations reached 0)\n  --> Colliding Stations: ");
            for (int i = 0; i < readyCount; i++) {
                printf("Station %d ", stations[ready[i]].id);
            }
            printf("\n  --> No ACK received (Timeout). Doubling Contention Window (CW):\n");

            for (int i = 0; i < readyCount; i++) {
                int idx = ready[i];
                stations[idx].attempts++;
                stations[idx].cw = min_int(stations[idx].cw * 2, 64);
                stations[idx].backoffSlots = rand() % stations[idx].cw;

                printf("      Station %d: New CW = %d, New Backoff = %d slots\n",
                       stations[idx].id, stations[idx].cw, stations[idx].backoffSlots);
            }
        }

        slot++;
    }

    printResults("CSMA/CA", stations, n, collisions, slot - 1);
}

// ============================================================================
// Main Interactive Menu
// ============================================================================
int main(void) {
    srand((unsigned int)time(NULL));
    int choice;

    while (1) {
        printf("\n====================================\n");
        printf("       CSMA CONSOLE SIMULATOR       \n");
        printf("====================================\n");
        printf("1. Simulate CSMA/CD (Collision Detection)\n");
        printf("2. Simulate CSMA/CA (Collision Avoidance)\n");
        printf("3. Exit\n");
        printf("Enter your choice (1-3): ");

        if (scanf("%d", &choice) != 1) {
            break;
        }
        clearInputBuffer();

        if (choice == 1) {
            simulateCSMACD();
        } else if (choice == 2) {
            simulateCSMACA();
        } else if (choice == 3) {
            printf("\nExiting. Thank you!\n");
            break;
        } else {
            printf("Invalid choice! Please enter a number between 1 and 3.\n");
        }
    }

    return 0;
}
