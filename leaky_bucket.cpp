/*
 * Leaky Bucket Algorithm - Traffic Shaping Simulation
 * ------------------------------------------------------
 * Model:
 *   - Packets of random (bursty) size arrive at the bucket at each
 *     discrete time tick, simulating irregular/bursty input traffic.
 *   - The bucket has a fixed CAPACITY (bytes). Incoming data is queued
 *     in the bucket.
 *       - If the bucket has enough free space, the packet is accepted.
 *       - If not, the packet (or the portion that doesn't fit) is
 *         DROPPED  -> this is how the algorithm enforces congestion
 *         control and protects the network from bursty senders.
 *   - The bucket "leaks" at a constant OUTPUT_RATE (bytes/tick),
 *     regardless of how bursty the input was. This is what performs
 *     TRAFFIC SHAPING: bursty input -> smooth, constant-rate output.
 *
 * The simulation prints a tick-by-tick trace and summary statistics
 * (packets accepted/dropped, bytes sent smoothly, average queue level).
 */

#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iomanip>
using namespace std;

/* --------------------- Configuration --------------------- */
const int BUCKET_CAPACITY   = 200;   // max bytes the bucket can hold
const int OUTPUT_RATE       = 20;    // constant leak/output rate (bytes/tick)
const int TOTAL_TICKS       = 30;    // duration of simulation
const int MAX_PACKET_SIZE   = 60;    // largest possible arriving packet (bytes)
const double ARRIVAL_PROB   = 0.75;  // probability a packet arrives in a tick

int main() {
    srand((unsigned) time(nullptr));

    int bucketLevel = 0;             // current bytes queued in the bucket
    long totalArrived = 0;
    long totalAccepted = 0;
    long totalDropped = 0;
    long totalSent = 0;
    long sumQueueLevel = 0;
    int packetsArrived = 0;
    int packetsFullyDropped = 0;

    cout << "================ Leaky Bucket Simulation ================\n";
    cout << "Bucket capacity     : " << BUCKET_CAPACITY << " bytes\n";
    cout << "Output (leak) rate  : " << OUTPUT_RATE << " bytes/tick (constant)\n";
    cout << "Max packet size     : " << MAX_PACKET_SIZE << " bytes\n";
    cout << "Arrival probability : " << ARRIVAL_PROB << " per tick\n";
    cout << "Total ticks         : " << TOTAL_TICKS << "\n";
    cout << "============================================================\n\n";

    cout << left
         << setw(6)  << "Tick"
         << setw(10) << "Arrival"
         << setw(12) << "Accepted"
         << setw(10) << "Dropped"
         << setw(12) << "BucketLvl"
         << setw(10) << "Output"
         << "Bucket [Fill]\n";

    for (int tick = 1; tick <= TOTAL_TICKS; tick++) {
        // ---------- 1. Packet arrival (bursty input) ----------
        int arrival = 0;
        bool packetArrives = ((double) rand() / RAND_MAX) < ARRIVAL_PROB;
        if (packetArrives) {
            arrival = (rand() % MAX_PACKET_SIZE) + 1;   // 1..MAX_PACKET_SIZE
            packetsArrived++;
        }
        totalArrived += arrival;

        // ---------- 2. Admission control (bucket capacity check) ----------
        int freeSpace = BUCKET_CAPACITY - bucketLevel;
        int accepted = min(arrival, freeSpace);
        int dropped  = arrival - accepted;

        if (dropped > 0) {
            packetsFullyDropped++;
        }

        bucketLevel += accepted;
        totalAccepted += accepted;
        totalDropped  += dropped;

        // ---------- 3. Leak: constant-rate output ----------
        int output = min(OUTPUT_RATE, bucketLevel);
        bucketLevel -= output;
        totalSent += output;

        sumQueueLevel += bucketLevel;

        // ---------- 4. Print trace row ----------
        int fillChars = (bucketLevel * 20) / BUCKET_CAPACITY; // 20-char bar
        string bar(fillChars, '#');
        bar += string(20 - fillChars, '.');

        cout << left
             << setw(6)  << tick
             << setw(10) << arrival
             << setw(12) << accepted
             << setw(10) << dropped
             << setw(12) << bucketLevel
             << setw(10) << output
             << "[" << bar << "]"
             << (dropped > 0 ? "  <-- OVERFLOW/DROP" : "")
             << "\n";
    }

    // ---------------- Summary ----------------
    cout << "\n============================ Summary ============================\n";
    cout << "Packets that arrived         : " << packetsArrived << "\n";
    cout << "Packets that suffered drop   : " << packetsFullyDropped << "\n";
    cout << "Total bytes arrived          : " << totalArrived << "\n";
    cout << "Total bytes accepted         : " << totalAccepted << "\n";
    cout << "Total bytes dropped          : " << totalDropped << "\n";
    cout << "Total bytes sent (leaked out): " << totalSent << "\n";
    cout << fixed << setprecision(2);
    cout << "Average bucket queue level   : "
         << (double) sumQueueLevel / TOTAL_TICKS << " bytes\n";
    cout << "Effective output rate        : "
         << (double) totalSent / TOTAL_TICKS << " bytes/tick "
         << "(target was " << OUTPUT_RATE << " bytes/tick)\n";
    if (totalArrived > 0) {
        cout << "Packet loss ratio            : "
             << (100.0 * totalDropped / totalArrived) << " %\n";
    }
    cout << "===================================================================\n";
    cout << "\nNote: The leak/output rate stays constant (" << OUTPUT_RATE
         << " bytes/tick) regardless of how\nbursty the input traffic is - "
            "this is the traffic-shaping effect of the algorithm.\nBytes "
            "that don't fit in the bucket are dropped, which is how the "
            "algorithm enforces\ncongestion control on the sender.\n";

    return 0;
}
