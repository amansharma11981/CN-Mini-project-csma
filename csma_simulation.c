#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

/*
 * ============================================================================
 * CSMA, CSMA/CD, and CSMA/CA Educational Mini Project in Pure C
 *
 * Implements:
 * 1. Pure CSMA (1-Persistent)
 * 2. CSMA/CD (Collision Detection, Jam Signal, Binary Exponential Backoff)
 * 3. CSMA/CA (Collision Avoidance, Random Backoff, Simplified RTS/CTS, ACK)
 *
 * Features:
 * - Simple, educational, single C file implementation.
 * - Complete user control over parameters (stations, retry limit, CW, RTS/CTS).
 * - Shared medium state tracking (IDLE vs BUSY).
 * - Full automated demo option.
 *
 * Compile:
 *   gcc csma_simulation.c -o csma_sim
 * ============================================================================
 */

// Shared medium state
typedef enum {
    MEDIUM_IDLE = 0,
    MEDIUM_BUSY = 1
} MediumState;

// Convert medium state to human-readable string
static inline const char* mediumStateStr(MediumState state) {
    return (state == MEDIUM_IDLE) ? "IDLE" : "BUSY";
}

// User input helper with bounds check and default fallback on [Enter]
static int getIntInput(const char* prompt, int minVal, int maxVal, int defaultVal) {
    char buf[64];
    printf("%s [%d-%d] (default %d): ", prompt, minVal, maxVal, defaultVal);
    fflush(stdout);

    if (!fgets(buf, sizeof(buf), stdin)) {
        return defaultVal;
    }
    // If user pressed Enter directly
    if (buf[0] == '\n' || buf[0] == '\r') {
        return defaultVal;
    }
    int val;
    if (sscanf(buf, "%d", &val) != 1) {
        printf("   [!] Invalid input. Using default: %d\n", defaultVal);
        return defaultVal;
    }
    if (val < minVal) {
        printf("   [!] Value %d below minimum. Set to %d\n", val, minVal);
        return minVal;
    }
    if (val > maxVal) {
        printf("   [!] Value %d exceeds maximum. Set to %d\n", val, maxVal);
        return maxVal;
    }
    return val;
}

// Interactive step-by-step pause
static void pauseStep(bool stepByStep) {
    if (!stepByStep) return;
    printf("   [Press Enter to continue...]");
    fflush(stdout);
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/*
 * ============================================================================
 * 1. Pure CSMA (1-Persistent)
 *
 * Rule:
 * - Sense the shared medium.
 * - If IDLE: Transmit immediately (probability p = 1).
 * - If BUSY: Listen continuously until IDLE, then transmit immediately (p = 1).
 * - Result: If multiple stations wait during a busy transmission, both transmit
 *   at the exact moment the medium becomes free -> COLLISION occurs.
 * ============================================================================
 */
void simulatePureCSMA(bool isInteractive) {
    printf("\n======================================================================\n");
    printf("                  1. PURE CSMA (1-PERSISTENT) SIMULATION               \n");
    printf("======================================================================\n");
    printf("Principle: Sense medium continuously; transmit immediately (p=1) when idle.\n\n");

    int numStations = 3;
    int txDuration = 3;
    bool stepByStep = false;

    if (isInteractive) {
        printf("--- Simulation Parameters (User Control) ---\n");
        numStations = getIntInput("Number of competing stations", 2, 6, 3);
        txDuration = getIntInput("Frame transmission duration in slots", 2, 6, 3);
        stepByStep = (getIntInput("Step-by-step mode? (1 = Yes, 0 = No)", 0, 1, 0) == 1);
        printf("\n");
    }

    MediumState medium = MEDIUM_IDLE;
    int currentSlot = 0;

    // Step 1: Initial transmission
    printf("[Time Slot %d] Medium State: %s\n", currentSlot, mediumStateStr(medium));
    printf("              -> Station 1 has a frame ready to transmit.\n");
    printf("              -> Station 1 senses medium: %s.\n", mediumStateStr(medium));
    printf("              -> Station 1 begins transmitting (Duration: %d slots).\n", txDuration);
    medium = MEDIUM_BUSY;
    printf("              -> Medium State updated: %s.\n", mediumStateStr(medium));
    pauseStep(stepByStep);

    // Step 2: Remaining stations arrive during transmission
    printf("\n[Time Slot %d to %d] Ongoing transmission by Station 1:\n",
           currentSlot + 1, currentSlot + txDuration - 1);
    for (int i = 2; i <= numStations; i++) {
        printf("              -> Station %d generates a frame at slot %d.\n", i, (currentSlot + (i - 1)));
        printf("              -> Station %d senses medium: %s.\n", i, mediumStateStr(medium));
        printf("              -> [1-Persistent Rule] Station %d continues listening continuously.\n", i);
    }
    pauseStep(stepByStep);

    // Step 3: Station 1 finishes
    currentSlot += txDuration;
    printf("\n[Time Slot %d] Station 1 finishes transmission.\n", currentSlot);
    printf("              -> Station 1 frame: DELIVERED SUCCESSFULLY!\n");
    medium = MEDIUM_IDLE;
    printf("              -> Medium State updated: %s.\n", mediumStateStr(medium));
    pauseStep(stepByStep);

    // Step 4: Vulnerability of 1-persistent
    printf("\n[Time Slot %d] Medium becomes IDLE:\n", currentSlot);
    printf("              -> All %d queued stations (Stations 2 through %d) sense IDLE at once.\n",
           numStations - 1, numStations);
    printf("              -> Because p = 1, ALL %d waiting stations transmit IMMEDIATELY!\n", numStations - 1);
    printf("              -> [!] COLLISION DETECTED between %d stations!\n", numStations - 1);
    printf("              -> All colliding frames are corrupted. Medium is jammed.\n");
    pauseStep(stepByStep);

    printf("\n--- Results Summary ---\n");
    printf("Total Stations:       %d\n", numStations);
    printf("Successful Delivery:  1 (Station 1)\n");
    printf("Collided Frames:      %d (Stations 2 to %d)\n", numStations - 1, numStations);
    printf("Success Rate:         %.1f%%\n", (1.0 / numStations) * 100.0);
    printf("\n>> Educational Insight: 1-Persistent CSMA has a high collision rate\n");
    printf("   because all stations that become ready during a busy transmission\n");
    printf("   wait for the channel to become idle and transmit simultaneously.\n");
    printf("======================================================================\n");
}

/*
 * ============================================================================
 * 2. CSMA/CD (Collision Detection — IEEE 802.3 Ethernet)
 *
 * Rule:
 * - Sense medium before transmitting (Carrier Sense).
 * - Listen while transmitting (Collision Detection).
 * - If collision detected:
 *     1. Abort transmission immediately.
 *     2. Broadcast Jamming signal to inform all network nodes.
 *     3. Apply Binary Exponential Backoff (BEB): R in [0, 2^k - 1] slots.
 * ============================================================================
 */
void simulateCSMACD(bool isInteractive) {
    printf("\n======================================================================\n");
    printf("            2. CSMA/CD (COLLISION DETECTION) SIMULATION               \n");
    printf("======================================================================\n");
    printf("Concept: Listen-while-talk + Jam Signal + Binary Exponential Backoff.\n\n");

    int numStations = 3;
    int maxAttempts = 10;
    int slotTimeMs = 10;
    bool stepByStep = false;

    if (isInteractive) {
        printf("--- Simulation Parameters (User Control) ---\n");
        numStations = getIntInput("Number of contending stations", 2, 6, 3);
        maxAttempts = getIntInput("Max collision retry attempts", 3, 16, 10);
        slotTimeMs = getIntInput("Slot time in ms", 5, 20, 10);
        stepByStep = (getIntInput("Step-by-step mode? (1 = Yes, 0 = No)", 0, 1, 0) == 1);
        printf("\n");
    }

    MediumState medium = MEDIUM_IDLE;

    // Step 1: Carrier sensing & concurrent start
    printf("[Step 1] Initial Carrier Sensing:\n");
    printf("         -> Medium State: %s.\n", mediumStateStr(medium));
    printf("         -> %d stations (", numStations);
    for (int i = 1; i <= numStations; i++) {
        printf("Station %d%s", i, (i == numStations ? "" : ", "));
    }
    printf(") all have frames ready.\n");
    printf("         -> All stations sense medium: IDLE at nearly the same instant.\n");
    printf("         -> All %d stations begin transmitting simultaneously!\n", numStations);
    medium = MEDIUM_BUSY;
    printf("         -> Medium State updated: %s.\n", mediumStateStr(medium));
    pauseStep(stepByStep);

    // Step 2: Collision detection & jam
    printf("\n[Step 2] Collision Detection (Listen-While-Talk):\n");
    printf("         -> Transceivers detect colliding voltage levels on the wire.\n");
    printf("         -> All %d stations ABORT transmission immediately (bandwidth saved)!\n", numStations);
    printf("         -> Broadcasting 32-bit JAM SIGNAL to inform all stations on the bus.\n");
    medium = MEDIUM_IDLE;
    printf("         -> Medium cleared: %s.\n", mediumStateStr(medium));
    pauseStep(stepByStep);

    // Step 3: Binary Exponential Backoff (Attempt k = 1)
    int k = 1;
    int maxR = (1 << k) - 1; // 2^1 - 1 = 1
    printf("\n[Step 3] Binary Exponential Backoff (Attempt k = %d):\n", k);
    printf("         -> Formula: R in [0, 2^min(k, 10) - 1] = [0, %d] slots.\n", maxR);
    printf("         -> Each station independently draws a random backoff R:\n");

    int backoffs[6];
    for (int i = 0; i < numStations; i++) {
        backoffs[i] = rand() % (maxR + 1);
        printf("            * Station %d: R = %d slot(s) -> wait = %d ms\n",
               i + 1, backoffs[i], backoffs[i] * slotTimeMs);
    }
    pauseStep(stepByStep);

    // Step 4: Resolution of contention
    int minB = backoffs[0];
    for (int i = 1; i < numStations; i++) {
        if (backoffs[i] < minB) minB = backoffs[i];
    }

    int minCount = 0;
    int minIndex = -1;
    for (int i = 0; i < numStations; i++) {
        if (backoffs[i] == minB) {
            minCount++;
            minIndex = i;
        }
    }

    printf("\n[Step 4] Timer Expiration & Channel Re-sensing:\n");
    if (minCount == 1) {
        printf("         -> Station %d had the lowest backoff timer (%d ms)!\n",
               minIndex + 1, minB * slotTimeMs);
        printf("         -> Station %d senses medium: %s.\n", minIndex + 1, mediumStateStr(medium));
        printf("         -> Station %d starts transmitting frame (Duration: 20 ms).\n", minIndex + 1);
        medium = MEDIUM_BUSY;
        printf("         -> Medium State: %s.\n", mediumStateStr(medium));
        printf("         -> Other stations wake up later, sense BUSY, and defer!\n");
        printf("         -> Station %d finishes: DELIVERED SUCCESSFULLY!\n", minIndex + 1);
        medium = MEDIUM_IDLE;
    } else {
        printf("         -> %d stations tied with minimum backoff R = %d slots!\n", minCount, minB);
        printf("         -> Subsequent attempt k = 2 expands contention window to [0, 3] slots.\n");
        printf("         -> Stations desynchronize on next backoff retry and transmit successfully!\n");
    }
    pauseStep(stepByStep);

    printf("\n--- Results Summary ---\n");
    printf("Contending Stations:  %d\n", numStations);
    printf("Collision Handled:    Immediate Jam signal + Backoff\n");
    printf("Max Retry Limit:      %d attempts\n", maxAttempts);
    printf("\n>> Educational Summary (CSMA/CD):\n");
    printf("   - Stations abort early on collision, saving significant bus time.\n");
    printf("   - Binary Exponential Backoff doubles the range each retry (up to 2^10),\n");
    printf("     rapidly resolving congestion even under heavy load.\n");
    printf("======================================================================\n");
}

/*
 * ============================================================================
 * 3. CSMA/CA (Collision Avoidance — IEEE 802.11 Wi-Fi)
 *
 * Rule:
 * - Wireless stations cannot easily listen while transmitting (half-duplex RF).
 * - Avoid collisions before transmitting:
 *     1. Sense medium for DIFS (Distributed Inter-Frame Space) period.
 *     2. Choose random backoff from Contention Window (CW).
 *     3. Exchange short control frames:
 *        - Sender transmits RTS (Request to Send).
 *        - Receiver replies with CTS (Clear to Send).
 *        - Other stations overhear CTS and update NAV (Network Allocation Vector).
 *     4. Sender transmits DATA frame.
 *     5. Receiver sends ACK (Acknowledgment) after SIFS.
 * ============================================================================
 */
void simulateCSMACA(bool isInteractive) {
    printf("\n======================================================================\n");
    printf("            3. CSMA/CA (COLLISION AVOIDANCE) SIMULATION               \n");
    printf("======================================================================\n");
    printf("Concept: Avoid-before-talk + Random Backoff + Simplified RTS/CTS + ACK.\n\n");

    int numStations = 2;
    int cw = 8;
    bool enableRtsCts = true;
    bool stepByStep = false;

    if (isInteractive) {
        printf("--- Simulation Parameters (User Control) ---\n");
        numStations = getIntInput("Number of wireless stations", 2, 5, 2);
        cw = getIntInput("Contention Window size (CW)", 4, 32, 8);
        enableRtsCts = (getIntInput("Enable RTS/CTS Handshake? (1 = Yes, 0 = No)", 0, 1, 1) == 1);
        stepByStep = (getIntInput("Step-by-step mode? (1 = Yes, 0 = No)", 0, 1, 0) == 1);
        printf("\n");
    }

    MediumState medium = MEDIUM_IDLE;

    // Step 1: DIFS Sensing
    printf("[Step 1] Carrier Sensing & DIFS Wait:\n");
    printf("         -> Medium State: %s.\n", mediumStateStr(medium));
    printf("         -> %d stations wish to transmit to the Access Point (AP).\n", numStations);
    printf("         -> All stations verify medium remains clear for DIFS (10 ms).\n");
    printf("         -> DIFS successfully observed without carrier intrusion.\n");
    pauseStep(stepByStep);

    // Step 2: Random Contention Window countdown
    printf("\n[Step 2] Contention Window Countdown (CW = %d):\n", cw);
    int backoffs[5];
    int minIndex = 0;
    for (int i = 0; i < numStations; i++) {
        backoffs[i] = 1 + (rand() % (cw - 1));
        printf("         -> Station %d picks random backoff: %d slots.\n", i + 1, backoffs[i]);
        if (backoffs[i] < backoffs[minIndex]) {
            minIndex = i;
        }
    }
    printf("         -> Station %d counter reaches 0 first (lowest backoff: %d slots)!\n",
           minIndex + 1, backoffs[minIndex]);
    pauseStep(stepByStep);

    if (enableRtsCts) {
        // Step 3: RTS
        printf("\n[Step 3] RTS (Request To Send) Handshake:\n");
        printf("         -> Station %d transmits short RTS control frame to AP.\n", minIndex + 1);
        medium = MEDIUM_BUSY;
        printf("         -> Medium State: %s (RTS reservation in progress).\n", mediumStateStr(medium));
        printf("         -> AP receives RTS intact.\n");
        pauseStep(stepByStep);

        // Step 4: CTS & NAV
        printf("\n[Step 4] CTS (Clear To Send) & Virtual Carrier Sense:\n");
        printf("         -> AP waits SIFS (5 ms) and broadcasts CTS frame.\n");
        printf("         -> Other stations (e.g. hidden terminals) overhear CTS.\n");
        printf("         -> Other stations update NAV (Network Allocation Vector) timer\n");
        printf("            and freeze transmission, eliminating hidden-terminal collisions!\n");
        pauseStep(stepByStep);

        // Step 5: DATA
        printf("\n[Step 5] DATA Frame Transmission:\n");
        printf("         -> Station %d waits SIFS and transmits full DATA payload.\n", minIndex + 1);
        printf("         -> Medium reserved exclusively: zero chance of data collision!\n");
        pauseStep(stepByStep);

        // Step 6: ACK
        printf("\n[Step 6] Acknowledgment (ACK):\n");
        printf("         -> AP receives DATA frame error-free.\n");
        printf("         -> AP waits SIFS and returns ACK frame.\n");
        medium = MEDIUM_IDLE;
        printf("         -> Station %d receives ACK: Delivery SUCCESSFUL & CONFIRMED!\n", minIndex + 1);
        printf("         -> Medium State restored: %s.\n", mediumStateStr(medium));
    } else {
        printf("\n[Step 3] Direct DATA Transmission (RTS/CTS Disabled):\n");
        printf("         -> Station %d sends DATA directly without channel reservation.\n", minIndex + 1);
        medium = MEDIUM_BUSY;
        printf("         -> AP receives data, waits SIFS, and returns ACK.\n");
        medium = MEDIUM_IDLE;
        printf("         -> Note: Without RTS/CTS, hidden terminals cannot be prevented\n");
        printf("            from transmitting and corrupting large data packets.\n");
    }
    pauseStep(stepByStep);

    printf("\n--- Results Summary ---\n");
    printf("Wireless Stations:    %d\n", numStations);
    printf("Contention Window:    %d slots\n", cw);
    printf("RTS/CTS Handshake:    %s\n", enableRtsCts ? "ENABLED (Protected via NAV)" : "DISABLED");
    printf("Frame Delivery:       CONFIRMED via ACK\n");
    printf("\n>> Educational Summary (CSMA/CA):\n");
    printf("   - Avoids collisions because wireless transceivers cannot listen while talking.\n");
    printf("   - Small RTS/CTS frames cheaply reserve the channel, so large data frames\n");
    printf("     never collide, even with hidden terminals.\n");
    printf("======================================================================\n");
}

/*
 * ============================================================================
 * Short Full Demo
 * Runs all three protocols sequentially with step-by-step commentary
 * ============================================================================
 */
void runFullDemo(void) {
    printf("\n######################################################################\n");
    printf("#            CSMA PROTOCOL FAMILY -- COMPREHENSIVE DEMO              #\n");
    printf("#     Demonstrating Pure CSMA, CSMA/CD, and CSMA/CA in Pure C         #\n");
    printf("######################################################################\n");

    simulatePureCSMA(false);
    simulateCSMACD(false);
    simulateCSMACA(false);

    printf("\n######################################################################\n");
    printf("#                     FULL DEMO EXECUTION COMPLETE                   #\n");
    printf("######################################################################\n");
}

/*
 * ============================================================================
 * Main Menu & Entry Point
 * ============================================================================
 */
int main(int argc, char* argv[]) {
    // Seed random number generator
    srand((unsigned int)time(NULL));

    // If --demo argument passed, run full demo directly
    if (argc > 1) {
        runFullDemo();
        return 0;
    }

    while (1) {
        printf("\n======================================================\n");
        printf("           CSMA / CD / CA CONSOLE SIMULATOR           \n");
        printf("======================================================\n");
        printf("1. Run Full Demo (All 3 Protocols with default params)\n");
        printf("2. Simulate Pure CSMA (1-Persistent) [User Controlled]\n");
        printf("3. Simulate CSMA/CD (Collision Detection) [User Controlled]\n");
        printf("4. Simulate CSMA/CA (Collision Avoidance & RTS/CTS) [User Controlled]\n");
        printf("5. Exit\n");

        int choice = getIntInput("Enter your choice", 1, 5, 1);

        if (choice == 1) {
            runFullDemo();
        } else if (choice == 2) {
            simulatePureCSMA(true);
        } else if (choice == 3) {
            simulateCSMACD(true);
        } else if (choice == 4) {
            simulateCSMACA(true);
        } else if (choice == 5) {
            printf("\nExiting CSMA Simulator. Thank you!\n");
            break;
        }
    }

    return 0;
}
