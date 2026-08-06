/*
    Stop-and-Wait ARQ (Automatic Repeat reQuest) over a Noisy Channel
    -------------------------------------------------------------------
    This simulates the classic Stop-and-Wait ARQ protocol (also called the
    "alternating bit protocol") operating over an unreliable, noisy channel
    where frames and acknowledgements can be corrupted or lost.

    Key features simulated:
      1. SEQUENCE NUMBERS  : Each frame carries a 1-bit sequence number (0/1)
                              that alternates, so the receiver can detect
                              duplicate frames (caused by lost ACKs).
      2. NOISY CHANNEL      : Both data frames and ACK frames can be lost or
                              corrupted in transit, each with an independent
                              probability (data loss rate, ack loss rate).
      3. TIMEOUT & RETRANSMISSION : If the sender does not receive a valid
                              ACK within the timeout period, it retransmits
                              the same frame (up to a maximum retry limit).
      4. DUPLICATE DETECTION : The receiver checks the sequence number of
                              each arriving frame; if it's a duplicate of the
                              last accepted frame, it is discarded but still
                              ACKed (in case the original ACK was lost).
      5. STATISTICS          : Reports how many frames were sent, how many
                              retransmissions occurred, and whether any frame
                              exceeded the maximum retry limit (protocol failure).

    Compile:  g++ -std=c++17 -O2 stop_and_wait_arq.cpp -o saw_arq
    Run:      ./saw_arq
*/

#include <bits/stdc++.h>
using namespace std;

class StopAndWaitARQ {
private:
    int totalFrames;
    double frameLossRate;   // probability a data frame is lost/corrupted in transit
    double ackLossRate;     // probability an ACK is lost/corrupted in transit
    int maxRetries;         // maximum retransmission attempts before giving up
    mt19937 rng;
    uniform_real_distribution<double> dist;

    int totalTransmissions = 0;
    int totalRetransmissions = 0;
    int framesFailed = 0;

    bool isLost(double probability) {
        return dist(rng) < probability;
    }

public:
    StopAndWaitARQ(int frames, double frameLoss, double ackLoss, int retries)
        : totalFrames(frames), frameLossRate(frameLoss), ackLossRate(ackLoss),
          maxRetries(retries), rng(random_device{}()), dist(0.0, 1.0) {}

    void run() {
        cout << "===== Stop-and-Wait ARQ over a Noisy Channel =====\n";
        cout << "Total data frames to send : " << totalFrames << "\n";
        cout << "Frame loss probability     : " << frameLossRate << "\n";
        cout << "ACK loss probability       : " << ackLossRate << "\n";
        cout << "Max retransmission attempts: " << maxRetries << "\n";
        cout << "------------------------------------------------------------\n";

        int expectedSeqAtReceiver = 0; // receiver expects this sequence number next
        int senderSeq = 0;             // sender's current sequence number (0 or 1)

        for (int frame = 0; frame < totalFrames; frame++) {
            bool delivered = false;
            int attempts = 0;

            cout << "\n--- Sending Frame #" << frame << " (Seq = " << senderSeq << ") ---\n";

            while (!delivered && attempts <= maxRetries) {
                attempts++;
                totalTransmissions++;
                if (attempts > 1) totalRetransmissions++;

                cout << "Sender: Transmitting Frame #" << frame
                     << " [Seq=" << senderSeq << "]  (attempt " << attempts << ")\n";

                // --- Channel step 1: does the data frame get lost/corrupted? ---
                if (isLost(frameLossRate)) {
                    cout << "        [Channel] Data frame LOST/CORRUPTED in transit.\n";
                    cout << "Sender: Timeout -> no ACK received -> will retransmit.\n";
                    continue; // go to next attempt
                }

                // --- Frame arrives correctly at receiver ---
                cout << "Receiver: Frame #" << frame << " [Seq=" << senderSeq
                     << "] arrived intact.\n";

                if (senderSeq == expectedSeqAtReceiver) {
                    cout << "Receiver: Sequence number matches expected ("
                         << expectedSeqAtReceiver << "). Accepting frame, passing to upper layer.\n";
                    expectedSeqAtReceiver = 1 - expectedSeqAtReceiver; // flip expected bit
                } else {
                    cout << "Receiver: DUPLICATE frame detected (expected Seq="
                         << expectedSeqAtReceiver << "). Discarding data, but re-ACKing.\n";
                }

                cout << "Receiver: Sending ACK for Seq=" << senderSeq << "\n";

                // --- Channel step 2: does the ACK get lost/corrupted? ---
                if (isLost(ackLossRate)) {
                    cout << "        [Channel] ACK LOST/CORRUPTED in transit.\n";
                    cout << "Sender: Timeout -> no ACK received -> will retransmit"
                            " (receiver already has this frame, will detect duplicate).\n";
                    continue; // go to next attempt
                }

                cout << "Sender: ACK for Seq=" << senderSeq << " received successfully.\n";
                delivered = true;
            }

            if (!delivered) {
                cout << "Sender: FRAME #" << frame
                     << " FAILED after " << maxRetries << " retransmission attempts! "
                        "Giving up (protocol failure / link down).\n";
                framesFailed++;
            } else {
                cout << "Result: Frame #" << frame << " delivered successfully in "
                     << attempts << " attempt(s).\n";
                senderSeq = 1 - senderSeq; // flip sequence number for next frame
            }
        }

        printStatistics();
    }

    void printStatistics() {
        cout << "\n============================================================\n";
        cout << "SIMULATION SUMMARY\n";
        cout << "============================================================\n";
        cout << "Total frames attempted        : " << totalFrames << "\n";
        cout << "Frames successfully delivered : " << (totalFrames - framesFailed) << "\n";
        cout << "Frames that failed completely : " << framesFailed << "\n";
        cout << "Total transmissions (incl. retx): " << totalTransmissions << "\n";
        cout << "Total retransmissions          : " << totalRetransmissions << "\n";
        double efficiency = (double)totalFrames / totalTransmissions * 100.0;
        cout << fixed << setprecision(2);
        cout << "Channel efficiency             : " << efficiency << "%  "
             << "(useful frames / total transmissions)\n";
        cout << "============================================================\n";
    }
};

int main() {
    // Configuration: 12 frames, 20% chance a data frame is lost,
    // 15% chance an ACK is lost, max 5 retransmission attempts per frame
    int numFrames     = 12;
    double frameLoss  = 0.20;
    double ackLoss    = 0.15;
    int maxRetries    = 5;

    StopAndWaitARQ arq(numFrames, frameLoss, ackLoss, maxRetries);
    arq.run();

    // Interactive mode
    cout << "\nRun a custom simulation? (y/n): ";
    char choice;
    cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        int n, retries;
        double fLoss, aLoss;

        cout << "Number of frames to send: ";
        cin >> n;
        cout << "Frame loss probability (0.0 - 1.0): ";
        cin >> fLoss;
        cout << "ACK loss probability (0.0 - 1.0): ";
        cin >> aLoss;
        cout << "Max retransmission attempts: ";
        cin >> retries;

        StopAndWaitARQ customArq(n, fLoss, aLoss, retries);
        customArq.run();
    }

    return 0;
}
