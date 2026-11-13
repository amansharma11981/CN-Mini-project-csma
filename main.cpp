#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <algorithm>

using namespace std;

// Structure to represent a station on the network
struct Station {
    int id;
    int attempts;         // Collision / retry counter
    int backoffSlots;     // Slots remaining to wait
    int cw;               // Contention Window (for CSMA/CA)
    bool transmitted;     // True if successfully sent
    bool active;          // Ready to transmit
};

// ============================================================================
// Helper Functions (Common logic to eliminate code repetition)
// ============================================================================

// Read integer within [minVal, maxVal] with fallback
int getIntInput(const string& prompt, int minVal, int maxVal, int defaultVal) {
    int val;
    cout << prompt;
    if (!(cin >> val)) {
        cin.clear();
        string dummy;
        cin >> dummy;
        return defaultVal;
    }
    return max(minVal, min(maxVal, val));
}

// Find indices of active, untransmitted stations whose backoffSlots == 0
vector<int> getReadyStations(const vector<Station>& stations) {
    vector<int> ready;
    for (size_t i = 0; i < stations.size(); i++) {
        if (stations[i].active && !stations[i].transmitted && stations[i].backoffSlots == 0) {
            ready.push_back(i);
        }
    }
    return ready;
}

// Decrement remaining backoff counters for active waiting stations
void decrementBackoffs(vector<Station>& stations, bool printStatus = false) {
    for (auto& st : stations) {
        if (st.active && !st.transmitted && st.backoffSlots > 0) {
            st.backoffSlots--;
            if (printStatus) {
                cout << "      Station " << st.id << " backoff remaining: " << st.backoffSlots << " slots\n";
            }
        }
    }
}

// Initialize station collection
vector<Station> initStations(int n, int cwInitial = 0) {
    vector<Station> stations(n);
    for (int i = 0; i < n; i++) {
        stations[i] = { i + 1, 0, (cwInitial > 0 ? rand() % cwInitial : 0), cwInitial, false, true };
        if (cwInitial > 0) {
            cout << "  Station " << stations[i].id << " -> CW: " << stations[i].cw 
                 << ", Initial Backoff: " << stations[i].backoffSlots << " slots\n";
        }
    }
    return stations;
}

// Print unified summary report
void printResults(const string& title, const vector<Station>& stations, int collisions, int slotsUsed) {
    int successful = 0;
    for (const auto& st : stations) {
        if (st.transmitted) successful++;
    }
    int total = stations.size();

    cout << "\n============================================\n";
    cout << "           " << title << " RESULTS                  \n";
    cout << "============================================\n";
    cout << "Total Stations:          " << total << "\n";
    cout << "Successfully Delivered:  " << successful << "\n";
    cout << "Frames Dropped:          " << (total - successful) << "\n";
    cout << "Total Collisions:        " << collisions << "\n";
    cout << "Total Time Slots Used:   " << slotsUsed << "\n";
    cout << "Success Rate:            " << (successful * 100.0 / total) << "%\n";
    cout << "============================================\n";
}

// ============================================================================
// 1. CSMA/CD (Collision Detection — IEEE 802.3 Ethernet)
// ============================================================================
void simulateCSMACD() {
    cout << "\n============================================\n";
    cout << "     CSMA/CD SIMULATION (IEEE 802.3)        \n";
    cout << "  Principle: Listen While Talk + Jam + BEB  \n";
    cout << "============================================\n";

    int n = getIntInput("Enter number of stations (2 to 10): ", 2, 10, 4);
    int maxRetries = getIntInput("Enter max collision attempts (default 10 or 16): ", 1, 16, 10);

    vector<Station> stations = initStations(n);
    int slot = 1, completed = 0, collisions = 0;

    cout << "\n--- Starting CSMA/CD Simulation ---\n";

    while (completed < n && slot <= 50) {
        cout << "\n[Time Slot " << slot << "]\n";
        vector<int> ready = getReadyStations(stations);

        if (ready.empty()) {
            cout << "  Channel Status: IDLE (Stations waiting in backoff)\n";
        } else if (ready.size() == 1) {
            int idx = ready[0];
            stations[idx].transmitted = true;
            stations[idx].active = false;
            completed++;
            cout << "  Channel Status: SUCCESS\n"
                 << "  --> Station " << stations[idx].id << " successfully transmitted its frame!\n";
        } else {
            collisions++;
            cout << "  Channel Status: COLLISION DETECTED!\n  --> Simultaneous transmission by: ";
            for (int idx : ready) cout << "Station " << stations[idx].id << " ";
            cout << "\n  --> Jamming signal broadcasted; transmission aborted.\n";

            for (int idx : ready) {
                stations[idx].attempts++;
                int k = stations[idx].attempts;

                if (k > maxRetries) {
                    cout << "  [!] Station " << stations[idx].id << " reached max retries (" 
                         << maxRetries << "). Frame DROPPED.\n";
                    stations[idx].active = false;
                    completed++;
                } else {
                    int maxR = (1 << min(k, 10)) - 1;
                    int r = rand() % (maxR + 1);
                    stations[idx].backoffSlots = r;

                    cout << "      Station " << stations[idx].id << " (Attempt " << k 
                         << "): Backoff R = " << r << " slots (range [0, " << maxR << "])\n";
                }
            }
        }

        decrementBackoffs(stations);
        slot++;
    }

    printResults("CSMA/CD", stations, collisions, slot - 1);
}

// ============================================================================
// 2. CSMA/CA (Collision Avoidance — IEEE 802.11 Wi-Fi)
// ============================================================================
void simulateCSMACA() {
    cout << "\n============================================\n";
    cout << "     CSMA/CA SIMULATION (IEEE 802.11)       \n";
    cout << "  Principle: Avoid Before Talk + DIFS/ACK   \n";
    cout << "============================================\n";

    int n = getIntInput("Enter number of wireless stations (2 to 8): ", 2, 8, 3);

    cout << "\nInitializing Station Contention Windows & Backoff Timers:\n";
    vector<Station> stations = initStations(n, 8); // cwMin = 8

    int slot = 1, completed = 0, collisions = 0;

    cout << "\n--- Starting CSMA/CA Slotted Contention Simulation ---\n";

    while (completed < n && slot <= 50) {
        cout << "\n[Time Slot " << slot << "]\n";
        vector<int> ready = getReadyStations(stations);

        if (ready.empty()) {
            cout << "  Medium Status: IDLE (DIFS sensed clear)\n"
                 << "  --> Active stations decrementing backoff counter:\n";
            decrementBackoffs(stations, true);
        } else if (ready.size() == 1) {
            int idx = ready[0];
            stations[idx].transmitted = true;
            stations[idx].active = false;
            completed++;

            cout << "  Medium Status: BUSY (Station " << stations[idx].id << " Transmitting)\n"
                 << "  --> Frame transmitted successfully over wireless medium.\n"
                 << "  --> SIFS interval observed; Receiver returned ACK frame.\n"
                 << "  --> Other stations sense carrier and FREEZE backoff counters:\n";

            for (const auto& st : stations) {
                if (st.active && !st.transmitted) {
                    cout << "      Station " << st.id << " frozen at " << st.backoffSlots << " slots\n";
                }
            }
        } else {
            collisions++;
            cout << "  Medium Status: COLLISION (Multiple stations reached 0)\n  --> Colliding Stations: ";
            for (int idx : ready) cout << "Station " << stations[idx].id << " ";
            cout << "\n  --> No ACK received (Timeout). Doubling Contention Window (CW):\n";

            for (int idx : ready) {
                stations[idx].attempts++;
                stations[idx].cw = min(stations[idx].cw * 2, 64);
                stations[idx].backoffSlots = rand() % stations[idx].cw;

                cout << "      Station " << stations[idx].id << ": New CW = " << stations[idx].cw 
                     << ", New Backoff = " << stations[idx].backoffSlots << " slots\n";
            }
        }

        slot++;
    }

    printResults("CSMA/CA", stations, collisions, slot - 1);
}

// ============================================================================
// 3. Persistence Methods Guide
// ============================================================================
void explainPersistence() {
    cout << "\n============================================\n";
    cout << "         CSMA PERSISTENCE METHODS           \n";
    cout << "============================================\n";
    cout << "1. 1-Persistent CSMA:\n"
         << "   - Senses channel continuously.\n"
         << "   - If IDLE: Transmits immediately (probability = 1).\n"
         << "   - If BUSY: Keeps sensing until idle, then transmits immediately.\n"
         << "   - Note: High collision chance if multiple stations are waiting.\n\n"
         << "2. Non-Persistent CSMA:\n"
         << "   - Senses channel.\n"
         << "   - If IDLE: Transmits immediately.\n"
         << "   - If BUSY: Waits a random backoff time before sensing again.\n"
         << "   - Note: Reduces collisions, but channel may remain idle longer.\n\n"
         << "3. p-Persistent CSMA (Used in slotted channels):\n"
         << "   - Senses channel.\n"
         << "   - If IDLE: Transmits with probability 'p'; waits next slot with (1-p).\n"
         << "   - If BUSY: Waits until idle and repeats the test.\n"
         << "   - Note: Balances throughput and collision probability.\n"
         << "============================================\n";
}

// ============================================================================
// Main Interactive Menu
// ============================================================================
int main() {
    srand(time(0));
    int choice;

    while (true) {
        cout << "\n====================================\n"
             << "       CSMA CONSOLE SIMULATOR       \n"
             << "====================================\n"
             << "1. Simulate CSMA/CD (Collision Detection)\n"
             << "2. Simulate CSMA/CA (Collision Avoidance)\n"
             << "3. View CSMA Persistence Methods\n"
             << "4. Exit\n"
             << "Enter your choice (1-4): ";

        if (!(cin >> choice)) break;

        if (choice == 1)      simulateCSMACD();
        else if (choice == 2) simulateCSMACA();
        else if (choice == 3) explainPersistence();
        else if (choice == 4) {
            cout << "\nExiting. Thank you!\n";
            break;
        } else {
            cout << "Invalid choice! Please enter a number between 1 and 4.\n";
        }
    }

    return 0;
}
