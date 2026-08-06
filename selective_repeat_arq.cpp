/*
 * Selective Repeat ARQ - Sliding Window Simulation
 * --------------------------------------------------
 * Key properties of Selective Repeat (as opposed to Go-Back-N):
 *   - Sender and receiver each maintain a window of size N.
 *   - Sequence numbers are drawn from a space of size >= 2N
 *     (here exactly 2N) to avoid ambiguity between old and new frames.
 *   - Receiver individually ACKs every correctly received frame,
 *     even if it arrives out of order, and BUFFERS it.
 *   - Only the specific frame(s) that are lost/timed-out are
 *     retransmitted - not the whole window (unlike Go-Back-N).
 *   - Receiver delivers frames to the upper layer only when they
 *     are contiguous, sliding its window forward as gaps are filled.
 *
 * The simulation models a lossy channel for both data frames and
 * ACKs, and uses a simple round-based timeout for retransmission.
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
using namespace std;

/* --------------------- Configuration --------------------- */
const int WINDOW_SIZE   = 4;          // N: sender & receiver window size
const int TOTAL_FRAMES  = 12;         // total frames to transmit
const int TIMEOUT       = 3;          // rounds to wait before retransmit
const double DATA_LOSS_PROB = 0.30;   // probability a data frame is lost
const double ACK_LOSS_PROB  = 0.20;   // probability an ACK is lost
const int MAX_ROUNDS     = 500;       // safety cap

bool isLost(double prob) {
    return ((double) rand() / RAND_MAX) < prob;
}

int main() {
    srand((unsigned) time(nullptr));

    // ---------------- Sender-side state ----------------
    vector<bool> senderAcked(TOTAL_FRAMES, false); // ACK received by sender?
    vector<bool> everSent(TOTAL_FRAMES, false);    // has been transmitted at least once?
    vector<int>  lastSendRound(TOTAL_FRAMES, -1);  // round of most recent transmission
    vector<int>  retryCount(TOTAL_FRAMES, 0);
    int senderBase = 0;

    // ---------------- Receiver-side state ----------------
    vector<bool> receiverHas(TOTAL_FRAMES, false); // correctly received & buffered
    vector<bool> delivered(TOTAL_FRAMES, false);   // handed to upper layer (in order)
    int receiverBase = 0;

    int round = 0;

    cout << "================ Selective Repeat ARQ Simulation ================\n";
    cout << "Window size (N)   : " << WINDOW_SIZE << "\n";
    cout << "Total frames      : " << TOTAL_FRAMES << "\n";
    cout << "Sequence space    : " << 2 * WINDOW_SIZE << " (>= 2N, required for SR)\n";
    cout << "Timeout           : " << TIMEOUT << " rounds\n";
    cout << "Data loss prob.   : " << DATA_LOSS_PROB << "\n";
    cout << "ACK loss prob.    : " << ACK_LOSS_PROB << "\n";
    cout << "===================================================================\n\n";

    while (receiverBase < TOTAL_FRAMES && round < MAX_ROUNDS) {
        round++;
        cout << "----- Round " << round << " | Sender window base=" << senderBase
             << " | Receiver window base=" << receiverBase << " -----\n";

        int windowEnd = min(senderBase + WINDOW_SIZE, TOTAL_FRAMES);

        // ============ 1. SENDER: transmit / retransmit ============
        for (int seq = senderBase; seq < windowEnd; seq++) {
            if (senderAcked[seq]) continue;   // nothing to do, already acked

            bool needSend = false;
            if (!everSent[seq]) {
                needSend = true;
            } else if (round - lastSendRound[seq] >= TIMEOUT) {
                needSend = true;
                retryCount[seq]++;
                cout << "  [TIMEOUT] Frame " << seq << " not acked -> retransmit (attempt #"
                     << (retryCount[seq] + 1) << ")\n";
            }

            if (needSend) {
                everSent[seq] = true;
                lastSendRound[seq] = round;

                cout << "  Sender -> Frame " << seq << " transmitted... ";
                if (!isLost(DATA_LOSS_PROB)) {
                    cout << "delivered to receiver.\n";
                    // ============ 2. CHANNEL + RECEIVER processing ============
                    if (seq >= receiverBase && seq < receiverBase + WINDOW_SIZE) {
                        if (!receiverHas[seq]) {
                            receiverHas[seq] = true;
                            cout << "    Receiver buffers frame " << seq
                                 << (seq == receiverBase ? " (in-order)" : " (out-of-order)")
                                 << ".\n";
                        } else {
                            cout << "    Receiver already had frame " << seq << " (duplicate).\n";
                        }
                        // Receiver sends ACK back
                        cout << "    Receiver -> ACK(" << seq << ") sent... ";
                        if (!isLost(ACK_LOSS_PROB)) {
                            senderAcked[seq] = true;
                            cout << "received by sender.\n";
                        } else {
                            cout << "LOST in transit.\n";
                        }
                    } else if (seq < receiverBase) {
                        // Old frame, already delivered earlier -- the sender's
                        // copy of our earlier ACK must have been lost.
                        // Re-send ACK so the sender can slide its window.
                        cout << "    Receiver already delivered frame " << seq
                             << " earlier; resending ACK... ";
                        if (!isLost(ACK_LOSS_PROB)) {
                            senderAcked[seq] = true;
                            cout << "received by sender.\n";
                        } else {
                            cout << "LOST in transit.\n";
                        }
                    } else {
                        cout << "    (Frame " << seq << " outside receiver window, discarded)\n";
                    }
                } else {
                    cout << "LOST in transit.\n";
                }
            }
        }

        // ============ 3. RECEIVER: deliver in-order frames ============
        while (receiverBase < TOTAL_FRAMES && receiverHas[receiverBase]) {
            delivered[receiverBase] = true;
            cout << "  >>> Receiver delivers Frame " << receiverBase << " to upper layer.\n";
            receiverBase++;
        }

        // ============ 4. SENDER: slide window ============
        while (senderBase < TOTAL_FRAMES && senderAcked[senderBase]) {
            senderBase++;
        }

        cout << endl;
    }

    // ---------------- Summary ----------------
    cout << "===================================================================\n";
    if (receiverBase == TOTAL_FRAMES) {
        cout << "All " << TOTAL_FRAMES << " frames delivered successfully in "
             << round << " rounds.\n\n";
    } else {
        cout << "Simulation stopped after MAX_ROUNDS (" << MAX_ROUNDS
             << ") without full delivery.\n\n";
    }

    cout << "Per-frame retransmission summary:\n";
    cout << left << setw(10) << "Frame" << setw(12) << "Retries" << "Status\n";
    for (int i = 0; i < TOTAL_FRAMES; i++) {
        cout << left << setw(10) << i << setw(12) << retryCount[i]
             << (delivered[i] ? "Delivered" : "NOT delivered") << "\n";
    }

    int totalRetries = 0;
    for (int r : retryCount) totalRetries += r;
    cout << "\nTotal retransmissions: " << totalRetries << "\n";

    return 0;
}
