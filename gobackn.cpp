// Go-Back-N ARQ Protocol Simulation (simplified, single process)
// -----------------------------------------------------------
// Concepts modeled:
//   - Sliding window of size N (sender can have up to N unacked frames "in flight")
//   - Cumulative ACKs (ACK k means "all frames up to k received correctly")
//   - Simulated frame loss (random)
//   - Timeout -> resend ALL frames from base (go-back-N behavior)
//
// Kept intentionally simple: single-threaded, step-by-step event loop,
// no real sockets/networking - just prints what would happen.

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

const int TOTAL_FRAMES   = 5;   // total frames sender wants to deliver
const int WINDOW_SIZE    = 2;    // Go-Back-N window size
const int TIMEOUT_TICKS  = 3;    // ticks to wait before declaring timeout
const double LOSS_PROB   = 0.2;  // probability a frame gets lost in transit

// Simulate sending a frame over an unreliable channel.
// Returns true if frame arrives successfully, false if lost.
bool channelDeliversFrame() {
    double r = (double)rand() / RAND_MAX;
    return r > LOSS_PROB;
}

int main() {
    srand((unsigned)time(nullptr));

    int base = 0;          // oldest un-acked frame (window's left edge)
    int nextSeqNum = 0;    // next frame to be sent (window's right edge, exclusive)
    int expectedSeqNum = 0; // receiver's next expected frame (cumulative)

    vector<bool> delivered(TOTAL_FRAMES, false); // track which frames receiver has accepted
    int timer = -1; // -1 means timer not running

    cout << "=== Go-Back-N Simulation ===\n";
    cout << "Total frames: " << TOTAL_FRAMES
         << " | Window size: " << WINDOW_SIZE
         << " | Timeout: " << TIMEOUT_TICKS << " ticks\n\n";

    int tick = 0;
    while (base < TOTAL_FRAMES) {
        tick++;
        cout << "--- Tick " << tick << " | base=" << base
             << " nextSeqNum=" << nextSeqNum << " ---\n";

        // 1) Sender sends any new frames that fit in the window
        while (nextSeqNum < TOTAL_FRAMES && nextSeqNum < base + WINDOW_SIZE) {
            cout << "  Sender: sending frame " << nextSeqNum << "\n";

            if (channelDeliversFrame()) {
                // Receiver only accepts frame if it's the one it expects (in-order)
                if (nextSeqNum == expectedSeqNum) {
                    delivered[nextSeqNum] = true;
                    cout << "    Receiver: accepted frame " << nextSeqNum
                         << ", sends ACK " << nextSeqNum << "\n";
                    expectedSeqNum++;
                } else {
                    cout << "    Receiver: out-of-order frame " << nextSeqNum
                         << " discarded (expected " << expectedSeqNum << ")\n";
                }
            } else {
                cout << "    [Frame " << nextSeqNum << " LOST in channel]\n";
            }

            // Start timer for base frame if not already running
            if (timer == -1) timer = 0;
            nextSeqNum++;
        }

        // 2) Process cumulative ACK: slide window forward past all delivered frames
        while (base < TOTAL_FRAMES && delivered[base]) {
            cout << "  Sender: window slides, frame " << base << " ACKed\n";
            base++;
            timer = (base == nextSeqNum) ? -1 : 0; // restart timer if more unacked frames remain
        }

        // 3) Timeout handling
        if (timer != -1) {
            timer++;
            if (timer >= TIMEOUT_TICKS) {
                cout << "  !! TIMEOUT waiting for frame " << base
                     << " -> Go-Back-N: resending frames " << base
                     << " to " << (nextSeqNum - 1) << "\n";
                nextSeqNum = base; // go back and resend the whole window
                timer = 0;
            }
        }

        cout << "\n";
        if (tick > 200) { // safety guard against infinite loop
            cout << "Stopping: too many ticks (safety limit).\n";
            break;
        }
    }

    cout << "=== All " << TOTAL_FRAMES << " frames delivered successfully! ===\n";
    return 0;
}
