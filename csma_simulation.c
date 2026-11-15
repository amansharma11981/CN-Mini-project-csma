#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
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

// Helper to clear standard input buffer
static void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

/*
 * ============================================================================
 * 1. Pure CSMA (1-Persistent)
 *
 * Rule:
 * - Sense the medium.
 * - If IDLE: Transmit immediately (probability p = 1).
 * - If BUSY: Listen continuously until IDLE, then transmit immediately (p = 1).
 * - Result: If multiple stations wait during a busy transmission, both transmit
 *   at the exact moment the medium becomes free -> COLLISION occurs.
 * ============================================================================
 */
void simulatePureCSMA(void) {
    printf("\n======================================================================\n");
    printf("                  1. PURE CSMA (1-PERSISTENT) SIMULATION               \n");
    printf("======================================================================\n");
    printf("Concept: Sense medium continuously; transmit immediately (p=1) when idle.\n\n");

    MediumState medium = MEDIUM_IDLE;

    // Step 1: Initial state
    printf("[Step 1] Medium State: %s\n", mediumStateStr(medium));
    printf("         -> Station 1 has data to send.\n");
    printf("         -> Station 1 senses medium: %s.\n", mediumStateStr(medium));
    printf("         -> Station 1 begins transmitting frame (Duration: 30 ms).\n");
    medium = MEDIUM_BUSY;
    printf("         -> Medium State updated: %s.\n\n", mediumStateStr(medium));

    // Step 2: Other stations arrive while channel is busy
    printf("[Step 2] While Station 1 is transmitting:\n");
    printf("         -> Station 2 has data ready and senses medium: %s.\n", mediumStateStr(medium));
    printf("         -> [1-Persistent Rule] Station 2 continues listening continuously.\n");
    printf("         -> Station 3 has data ready and senses medium: %s.\n", mediumStateStr(medium));
    printf("         -> [1-Persistent Rule] Station 3 continues listening continuously.\n\n");

    // Step 3: Station 1 completes
    printf("[Step 3] Station 1 finishes transmission.\n");
    printf("         -> Station 1 transmission: SUCCESSFUL!\n");
    medium = MEDIUM_IDLE;
    printf("         -> Medium State updated: %s.\n\n", mediumStateStr(medium));

    // Step 4: Vulnerability of 1-Persistent
    printf("[Step 4] Both Station 2 and Station 3 sense the medium become IDLE.\n");
    printf("         -> Because probability p = 1, BOTH stations transmit IMMEDIATELY!\n");
    printf("         -> [!] COLLISION DETECTED between Station 2 and Station 3!\n");
    printf("         -> Frames are corrupted and cannot be decoded by receiver.\n");
    printf("         -> Both stations must wait and re-attempt.\n\n");

    printf(">> Educational Summary (Pure 1-Persistent CSMA):\n");
    printf("   - High channel utilization when network load is low.\n");
    printf("   - Prone to collisions when multiple stations wait for an ongoing\n");
    printf("     transmission to finish and all transmit at once.\n");
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
void simulateCSMACD(void) {
    printf("\n======================================================================\n");
    printf("            2. CSMA/CD (COLLISION DETECTION) SIMULATION               \n");
    printf("======================================================================\n");
    printf("Concept: Listen-while-talk + Jam Signal + Binary Exponential Backoff.\n\n");

    MediumState medium = MEDIUM_IDLE;

    // Step 1: Concurrent transmission
    printf("[Step 1] Medium State: %s\n", mediumStateStr(medium));
    printf("         -> Station A and Station B both have data ready.\n");
    printf("         -> Both sense medium: IDLE at nearly the same instant.\n");
    printf("         -> Both begin transmitting simultaneously!\n");
    medium = MEDIUM_BUSY;
    printf("         -> Medium State updated: %s.\n\n", mediumStateStr(medium));

    // Step 2: Collision detection & Jam signal
    printf("[Step 2] Collision Detected (Listen-While-Talk):\n");
    printf("         -> Transceivers detect overlapping signal voltages.\n");
    printf("         -> Transmissions ABORTED immediately (saves channel bandwidth)!\n");
    printf("         -> Transmitting 32-bit JAM SIGNAL to alert all network stations.\n");
    medium = MEDIUM_IDLE;
    printf("         -> Medium cleared: %s.\n\n", mediumStateStr(medium));

    // Step 3: Binary Exponential Backoff
    printf("[Step 3] Binary Exponential Backoff (Attempt k = 1):\n");
    printf("         -> Contention range: [0, 2^k - 1] = [0, 1] slot (Slot = 10 ms).\n");

    // Randomize slot selection (ensure demonstration shows separation)
    int slotTimeMs = 10;
    int backoffA = rand() % 2;
    int backoffB = rand() % 2;
    if (backoffA == backoffB) {
        backoffB = 1 - backoffA; // Distinct backoffs for clean educational trace
    }

    printf("         -> Station A chooses backoff: %d slot(s) = %d ms wait.\n",
           backoffA, backoffA * slotTimeMs);
    printf("         -> Station B chooses backoff: %d slot(s) = %d ms wait.\n\n",
           backoffB, backoffB * slotTimeMs);

    // Step 4: Re-transmission resolution
    char winner = (backoffA < backoffB) ? 'A' : 'B';
    char loser  = (backoffA < backoffB) ? 'B' : 'A';
    int waitWinner = (backoffA < backoffB) ? backoffA : backoffB;
    int waitLoser  = (backoffA < backoffB) ? backoffB : backoffA;

    printf("[Step 4] Station %c backoff timer (%d ms) expires first:\n", winner, waitWinner * slotTimeMs);
    printf("         -> Station %c senses medium: %s.\n", winner, mediumStateStr(medium));
    printf("         -> Station %c transmits frame (Duration: 20 ms).\n", winner);
    medium = MEDIUM_BUSY;
    printf("         -> Medium State: %s.\n\n", mediumStateStr(medium));

    printf("[Step 5] Station %c backoff timer (%d ms) expires:\n", loser, waitLoser * slotTimeMs);
    printf("         -> Station %c senses medium: %s (occupied by Station %c).\n",
           loser, mediumStateStr(medium), winner);
    printf("         -> Station %c defers transmission and waits for medium to clear.\n\n", loser);

    printf("[Step 6] Station %c completes transmission successfully!\n", winner);
    medium = MEDIUM_IDLE;
    printf("         -> Medium State updated: %s.\n", mediumStateStr(medium));
    printf("         -> Station %c senses IDLE and transmits its frame successfully!\n\n", loser);

    printf(">> Educational Summary (CSMA/CD):\n");
    printf("   - Aborts wasted transmissions early through continuous collision monitoring.\n");
    printf("   - Jam signal ensures all network nodes recognize the collision.\n");
    printf("   - Exponential backoff desynchronizes stations to prevent repeat collisions.\n");
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
void simulateCSMACA(void) {
    printf("\n======================================================================\n");
    printf("            3. CSMA/CA (COLLISION AVOIDANCE) SIMULATION               \n");
    printf("======================================================================\n");
    printf("Concept: Avoid-before-talk + Random Backoff + Simplified RTS/CTS + ACK.\n\n");

    MediumState medium = MEDIUM_IDLE;

    // Step 1: Carrier sense & DIFS
    printf("[Step 1] Medium State: %s\n", mediumStateStr(medium));
    printf("         -> Station X has data destined for Access Point (AP).\n");
    printf("         -> Station X senses medium: %s.\n", mediumStateStr(medium));
    printf("         -> Station X waits DIFS (Distributed Inter-Frame Space = 10 ms).\n");
    printf("         -> Medium remains idle during DIFS.\n\n");

    // Step 2: Random Contention Window backoff
    int cw = 8;
    int backoffSlots = 1 + (rand() % (cw - 1));
    printf("[Step 2] Contention Window (CW = %d):\n", cw);
    printf("         -> Station X picks random backoff: %d slots.\n", backoffSlots);
    printf("         -> Decrementing backoff counter while medium stays idle...\n");
    printf("         -> Backoff reached 0 slots without interruption.\n\n");

    // Step 3: RTS (Request to Send)
    printf("[Step 3] RTS Handshake (Channel Reservation):\n");
    printf("         -> Station X sends short RTS (Request To Send) control frame to AP.\n");
    medium = MEDIUM_BUSY;
    printf("         -> Medium State: %s (RTS in transit).\n", mediumStateStr(medium));
    printf("         -> AP receives RTS intact.\n\n");

    // Step 4: CTS (Clear to Send)
    printf("[Step 4] CTS Handshake:\n");
    printf("         -> AP waits SIFS (Short Inter-Frame Space = 5 ms).\n");
    printf("         -> AP broadcasts CTS (Clear To Send) frame with duration field.\n");
    printf("         -> Nearby Station Y overhears CTS -> Sets NAV (Network Allocation Vector)\n");
    printf("            and defers any transmission to solve the hidden-terminal problem!\n\n");

    // Step 5: DATA transmission
    printf("[Step 5] DATA Frame Transmission:\n");
    printf("         -> Station X waits SIFS after CTS.\n");
    printf("         -> Station X transmits full DATA payload safely.\n");
    printf("         -> Zero collisions because channel was reserved by RTS/CTS!\n\n");

    // Step 6: ACK
    printf("[Step 6] Acknowledgment (ACK):\n");
    printf("         -> AP receives DATA without errors.\n");
    printf("         -> AP waits SIFS and returns ACK frame.\n");
    medium = MEDIUM_IDLE;
    printf("         -> Station X receives ACK: Transmission SUCCESSFUL & CONFIRMED!\n");
    printf("         -> Medium State restored: %s.\n\n", mediumStateStr(medium));

    printf(">> Educational Summary (CSMA/CA):\n");
    printf("   - Avoids collisions instead of detecting them after the fact.\n");
    printf("   - RTS/CTS reservation protects large data frames from expensive collisions.\n");
    printf("   - NAV and SIFS/DIFS timings coordinate shared medium access cleanly.\n");
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

    simulatePureCSMA();
    simulateCSMACD();
    simulateCSMACA();

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

    int choice;
    while (1) {
        printf("\n======================================================\n");
        printf("           CSMA / CD / CA CONSOLE SIMULATOR           \n");
        printf("======================================================\n");
        printf("1. Run Full Demo (All 3 Protocols sequentially)\n");
        printf("2. Simulate Pure CSMA (1-Persistent)\n");
        printf("3. Simulate CSMA/CD (Collision Detection)\n");
        printf("4. Simulate CSMA/CA (Collision Avoidance & RTS/CTS)\n");
        printf("5. Exit\n");
        printf("Enter your choice (1-5): ");
        fflush(stdout);

        if (scanf("%d", &choice) != 1) {
            // If EOF or non-interactive pipe, run demo and exit
            printf("\nRunning full demo...\n");
            runFullDemo();
            break;
        }
        clearInputBuffer();

        if (choice == 1) {
            runFullDemo();
        } else if (choice == 2) {
            simulatePureCSMA();
        } else if (choice == 3) {
            simulateCSMACD();
        } else if (choice == 4) {
            simulateCSMACA();
        } else if (choice == 5) {
            printf("\nExiting CSMA Simulator. Thank you!\n");
            break;
        } else {
            printf("Invalid selection! Please enter a choice between 1 and 5.\n");
        }
    }

    return 0;
}
