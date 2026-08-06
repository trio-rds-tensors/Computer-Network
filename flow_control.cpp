/*
    Flow Control Protocols Simulation
    -------------------------------------------------------------
    1. STOP-AND-WAIT PROTOCOL
       - Sender transmits one frame and waits for its ACK before
         sending the next frame.
       - If a frame or its ACK is lost, a timeout occurs and the
         sender retransmits the same frame.

    2. SLIDING WINDOW PROTOCOL (Go-Back-N)
       - Sender can transmit up to W frames (window size W) without
         waiting for individual ACKs.
       - Receiver sends cumulative ACKs (ACK n = "I have received
         everything up to frame n, expecting n+1 next").
       - If a frame is lost, the receiver discards all frames that
         arrive after it (out of order) and the sender must go back
         and retransmit starting from the lost frame (Go-Back-N).

    Compile:  g++ -std=c++17 -O2 flow_control.cpp -o flow_control
    Run:      ./flow_control
*/

#include <bits/stdc++.h>
using namespace std;

void printSeparator() {
    cout << "----------------------------------------------------------\n";
}

/* ======================================================================
   1. STOP-AND-WAIT PROTOCOL
   ====================================================================== */
class StopAndWait {
private:
    int totalFrames;
    set<int> lostFrames;   // frame numbers that will be lost in transit (simulated)
    set<int> lostAcks;     // ACK numbers that will be lost in transit (simulated)

public:
    StopAndWait(int frames, set<int> loseFrames, set<int> loseAcks)
        : totalFrames(frames), lostFrames(loseFrames), lostAcks(loseAcks) {}

    void run() {
        cout << "\n===== STOP-AND-WAIT PROTOCOL SIMULATION =====\n";
        cout << "Total frames to send : " << totalFrames << "\n";
        printSeparator();

        for (int frame = 0; frame < totalFrames; frame++) {
            bool acked = false;
            int attempt = 0;

            while (!acked) {
                attempt++;
                cout << "Sender: Transmitting Frame " << frame
                     << " (attempt " << attempt << ")\n";

                // Check if this frame gets lost in the channel
                if (lostFrames.count(frame) && attempt == 1) {
                    cout << "        --> Frame " << frame << " LOST in transit!\n";
                    cout << "Sender: Timeout waiting for ACK " << frame
                         << " -> Retransmitting...\n";
                    continue;
                }

                cout << "Receiver: Frame " << frame << " received successfully.\n";
                cout << "Receiver: Sending ACK " << frame << "\n";

                // Check if the ACK gets lost on the way back
                if (lostAcks.count(frame) && attempt == 1) {
                    cout << "        --> ACK " << frame << " LOST in transit!\n";
                    cout << "Sender: Timeout waiting for ACK " << frame
                         << " -> Retransmitting...\n";
                    continue;
                }

                cout << "Sender: ACK " << frame << " received. Moving to next frame.\n";
                acked = true;
            }
            printSeparator();
        }

        cout << "All " << totalFrames << " frames delivered successfully.\n";
    }
};

/* ======================================================================
   2. SLIDING WINDOW PROTOCOL (GO-BACK-N)
   ====================================================================== */
class SlidingWindowGBN {
private:
    int totalFrames;
    int windowSize;
    set<int> lostFrames;   // frame numbers that will be lost (simulated, once)

public:
    SlidingWindowGBN(int frames, int window, set<int> loseFrames)
        : totalFrames(frames), windowSize(window), lostFrames(loseFrames) {}

    void run() {
        cout << "\n===== SLIDING WINDOW PROTOCOL (GO-BACK-N) SIMULATION =====\n";
        cout << "Total frames to send : " << totalFrames << "\n";
        cout << "Window size          : " << windowSize << "\n";
        printSeparator();

        int base = 0;              // oldest unacknowledged frame
        int nextToSend = 0;        // next frame to be sent
        set<int> alreadyLost;      // frames we've already simulated losing (avoid infinite loop)
        int round = 0;

        while (base < totalFrames) {
            round++;
            cout << "\n--- Round " << round << " | Window covers frames ["
                 << base << " .. " << min(base + windowSize - 1, totalFrames - 1) << "] ---\n";

            // Sender transmits all frames currently within the window
            int windowEnd = min(base + windowSize, totalFrames);
            bool lossOccurredThisRound = false;
            int lostAt = -1;

            for (int f = base; f < windowEnd; f++) {
                cout << "Sender: Transmitting Frame " << f << "\n";

                if (lostFrames.count(f) && !alreadyLost.count(f)) {
                    cout << "        --> Frame " << f << " LOST in transit!\n";
                    alreadyLost.insert(f);
                    lossOccurredThisRound = true;
                    lostAt = f;
                    break; // stop sending further frames this round (they will be discarded anyway)
                }
            }

            if (lossOccurredThisRound) {
                cout << "Receiver: Frame " << lostAt
                     << " never arrived. All subsequent frames in this round are discarded"
                        " (out of order).\n";
                cout << "Receiver: Re-sending cumulative ACK " << (lostAt - 1)
                     << " (last correctly received, in-order frame).\n";
                cout << "Sender: Timeout on Frame " << lostAt
                     << " -> GO-BACK-N: retransmitting from Frame " << lostAt << " onward.\n";
                base = lostAt; // go back and resend from the lost frame
                nextToSend = base;
                continue;
            }

            // No loss this round: all frames in the window were received correctly
            for (int f = base; f < windowEnd; f++) {
                cout << "Receiver: Frame " << f << " received successfully.\n";
            }
            int lastAck = windowEnd - 1;
            cout << "Receiver: Sending cumulative ACK " << lastAck << "\n";
            cout << "Sender: ACK " << lastAck << " received. Sliding window forward.\n";

            base = windowEnd;
            nextToSend = base;
        }

        printSeparator();
        cout << "All " << totalFrames << " frames delivered successfully.\n";
    }
};

int main() {
    // ------------------------------------------------------------
    // 1. Stop-and-Wait demo
    //    Frame 2 will be lost on first attempt, ACK 4 will be lost too
    // ------------------------------------------------------------
    StopAndWait saw(6, {2}, {4});
    saw.run();

    printSeparator();
    printSeparator();

    // ------------------------------------------------------------
    // 2. Sliding Window (Go-Back-N) demo
    //    Window size = 4, total 10 frames, Frame 5 gets lost once
    // ------------------------------------------------------------
    SlidingWindowGBN gbn(10, 4, {5});
    gbn.run();

    return 0;
}
