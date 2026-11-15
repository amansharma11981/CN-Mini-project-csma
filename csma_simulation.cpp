#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cstdlib>
#include <ctime>

using namespace std;

// Safe input helper with bounds checking and default value on Enter
int getIntInput(const string& prompt, int minVal, int maxVal, int defaultVal) {
    cout << prompt << " [" << minVal << "-" << maxVal << "] (default " << defaultVal << "): ";
    string line;
    if (!getline(cin, line) || line.empty()) {
        return defaultVal;
    }
    stringstream ss(line);
    int val;
    if (!(ss >> val) || val < minVal || val > maxVal) {
        return defaultVal;
    }
    return val;
}

// 1. Pure CSMA (1-Persistent)
void simulateCSMA() {
    cout << "\n======================================================\n";
    cout << "         1. PURE CSMA (1-PERSISTENT) SIMULATION       \n";
    cout << "======================================================\n";
    cout << "Rule: Sense channel. If idle, transmit. If busy, wait until idle and transmit immediately.\n\n";

    int stations = getIntInput("Number of competing stations", 2, 6, 3);

    cout << "\n[Step 1] Channel is IDLE.\n";
    cout << "         Station 1 senses medium and begins transmitting.\n";
    cout << "         Channel is now BUSY.\n";

    cout << "\n[Step 2] Stations 2 to " << stations << " become ready while channel is BUSY.\n";
    cout << "         1-Persistent rule: they wait and keep listening continuously.\n";

    cout << "\n[Step 3] Station 1 finishes transmission.\n";
    cout << "         Station 1 frame: DELIVERED SUCCESSFULLY!\n";
    cout << "         Channel becomes IDLE.\n";

    cout << "\n[Step 4] Vulnerability of 1-Persistent:\n";
    cout << "         Stations 2 to " << stations << " all sense channel IDLE at the same instant.\n";
    cout << "         Because p = 1, ALL waiting stations transmit IMMEDIATELY!\n";
    cout << "         --> COLLISION DETECTED! All colliding frames are corrupted.\n";

    cout << "\n--- Result ---\n";
    cout << "Summary: 1-Persistent CSMA causes collisions whenever multiple stations wait for a busy channel.\n";
}

// 2. CSMA/CD (Collision Detection - Ethernet)
void simulateCSMACD() {
    cout << "\n======================================================\n";
    cout << "       2. CSMA/CD (COLLISION DETECTION) SIMULATION    \n";
    cout << "======================================================\n";
    cout << "Rule: Listen while transmitting. If collision occurs, abort, send Jam Signal, and back off.\n\n";

    int stations = getIntInput("Number of contending stations", 2, 6, 3);

    cout << "\n[Step 1] Initial Carrier Sensing:\n";
    cout << "         All " << stations << " stations sense channel IDLE at the same time.\n";
    cout << "         All " << stations << " stations start transmitting simultaneously!\n";

    cout << "\n[Step 2] Collision Detection (Listen-While-Talk):\n";
    cout << "         Transceivers detect collision on the wire.\n";
    cout << "         All stations ABORT immediately to save bandwidth.\n";
    cout << "         A 32-bit JAM SIGNAL is broadcast to inform everyone.\n";

    cout << "\n[Step 3] Binary Exponential Backoff (Attempt 1):\n";
    cout << "         Each station draws a random backoff R in [0, 1] slot:\n";

    vector<int> backoff(stations);
    int minBackoff = 999;
    int minStation = 1;
    int tieCount = 0;

    for (int i = 0; i < stations; i++) {
        backoff[i] = rand() % 2; // 0 or 1 slot
        cout << "         * Station " << (i + 1) << " backoff = " << backoff[i] << " slot(s)\n";
        if (backoff[i] < minBackoff) {
            minBackoff = backoff[i];
            minStation = i + 1;
            tieCount = 1;
        } else if (backoff[i] == minBackoff) {
            tieCount++;
        }
    }

    cout << "\n[Step 4] Resolution:\n";
    if (tieCount == 1) {
        cout << "         Station " << minStation << " had the lowest backoff timer!\n";
        cout << "         Station " << minStation << " senses channel IDLE and transmits: DELIVERED SUCCESSFULLY!\n";
    } else {
        cout << "         " << tieCount << " stations tied with minimum backoff " << minBackoff << " slots.\n";
        cout << "         Next attempt expands backoff range to [0, 3] slots and resolves collision.\n";
    }

    cout << "\n--- Result ---\n";
    cout << "Summary: CSMA/CD aborts collisions early and uses exponential backoff to resolve congestion.\n";
}

// 3. CSMA/CA (Collision Avoidance - Wi-Fi)
void simulateCSMACA() {
    cout << "\n======================================================\n";
    cout << "       3. CSMA/CA (COLLISION AVOIDANCE) SIMULATION    \n";
    cout << "======================================================\n";
    cout << "Rule: Avoid collisions before transmitting using DIFS, random backoff, RTS/CTS, and ACK.\n\n";

    int stations = getIntInput("Number of wireless stations", 2, 5, 2);
    int useRtsCts = getIntInput("Enable RTS/CTS Handshake? (1 = Yes, 0 = No)", 0, 1, 1);

    cout << "\n[Step 1] Carrier Sensing:\n";
    cout << "         All " << stations << " stations verify channel stays clear for DIFS period (10 ms).\n";

    cout << "\n[Step 2] Contention Window Countdown:\n";
    vector<int> backoff(stations);
    int winner = 1;
    int minTimer = 999;
    for (int i = 0; i < stations; i++) {
        backoff[i] = 1 + (rand() % 7); // Random backoff 1-7 slots
        cout << "         * Station " << (i + 1) << " timer = " << backoff[i] << " slots\n";
        if (backoff[i] < minTimer) {
            minTimer = backoff[i];
            winner = i + 1;
        }
    }
    cout << "         Station " << winner << " countdown reaches 0 first!\n";

    if (useRtsCts == 1) {
        cout << "\n[Step 3] RTS/CTS Reservation Handshake:\n";
        cout << "         Station " << winner << " transmits RTS (Request to Send) to Access Point.\n";
        cout << "         Access Point replies with CTS (Clear to Send).\n";
        cout << "         Other stations overhear CTS and update NAV timer to freeze transmission.\n";
        cout << "         (Hidden terminal collision prevented!)\n";

        cout << "\n[Step 4] Data Transmission & ACK:\n";
        cout << "         Station " << winner << " transmits DATA frame securely.\n";
        cout << "         Access Point returns ACK: Delivery CONFIRMED!\n";
    } else {
        cout << "\n[Step 3] Direct Transmission (RTS/CTS Disabled):\n";
        cout << "         Station " << winner << " transmits DATA directly without channel reservation.\n";
        cout << "         Access Point returns ACK: Delivery CONFIRMED!\n";
    }

    cout << "\n--- Result ---\n";
    cout << "Summary: CSMA/CA avoids collisions wirelessly where hardware cannot detect collisions while transmitting.\n";
}

int main() {
    srand(static_cast<unsigned int>(time(nullptr)));

    while (true) {
        cout << "\n======================================================\n";
        cout << "           CSMA / CD / CA CONSOLE SIMULATOR           \n";
        cout << "======================================================\n";
        cout << "1. Simulate Pure CSMA (1-Persistent)\n";
        cout << "2. Simulate CSMA/CD (Collision Detection)\n";
        cout << "3. Simulate CSMA/CA (Collision Avoidance & RTS/CTS)\n";
        cout << "4. Exit\n";

        int choice = getIntInput("Enter your choice", 1, 4, 1);

        if (choice == 1) {
            simulateCSMA();
        } else if (choice == 2) {
            simulateCSMACD();
        } else if (choice == 3) {
            simulateCSMACA();
        } else if (choice == 4) {
            cout << "\nExiting CSMA Simulator. Thank you!\n";
            break;
        }
    }

    return 0;
}
