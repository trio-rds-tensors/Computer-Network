/*
    Cyclic Redundancy Check (CRC) over a Noisy Channel - Simulation
    -------------------------------------------------------------------
    Generator Polynomial : x^16 + x^12 + x^5 + 1   (CRC-CCITT, binary: 10001000000100001)
    Method                : Standard modulo-2 (XOR) binary polynomial division

    What this program simulates:
      1. SENDER    : Takes data bits, computes the CRC checksum, and forms
                      the transmitted frame (data + CRC).
      2. CHANNEL   : A noisy channel is simulated where each bit of the frame
                      has an independent probability (bit error rate, BER) of
                      being flipped during transmission.
      3. RECEIVER  : Recomputes the CRC over the received (possibly corrupted)
                      frame. If the remainder is nonzero, an error is detected
                      and the frame is discarded; otherwise it is accepted.
      4. STATISTICS: The simulation repeats the transmission many times over
                      the noisy channel and reports how many corrupted frames
                      were correctly detected versus how many slipped through
                      undetected (an undetected error means the corruption
                      pattern happened to be a multiple of the generator
                      polynomial, which CRC cannot catch).

    Compile:  g++ -std=c++17 -O2 crc_noisy_channel.cpp -o crc_noisy
    Run:      ./crc_noisy
*/

#include <bits/stdc++.h>
using namespace std;

// CRC-CCITT generator polynomial: x^16 + x^12 + x^5 + 1 (17 bits, degree 16)
const string GENERATOR = "10001000000100001";

class CRC {
public:
    // XOR of two equal-length binary strings, skipping the leading bit
    static string xorBits(const string &a, const string &b) {
        string result;
        for (size_t i = 1; i < a.size(); i++)
            result += (a[i] == b[i]) ? '0' : '1';
        return result;
    }

    // Modulo-2 binary division. Returns the remainder.
    static string mod2Divide(const string &dividend, const string &divisor) {
        int genLen = divisor.size();
        string current = dividend.substr(0, genLen);

        for (size_t i = genLen; i < dividend.size(); i++) {
            if (current[0] == '1')
                current = xorBits(divisor, current) + dividend[i];
            else
                current = xorBits(string(genLen, '0'), current) + dividend[i];
        }

        if (current[0] == '1')
            current = xorBits(divisor, current);
        else
            current = xorBits(string(genLen, '0'), current);

        return current; // 16-bit remainder for CRC-CCITT
    }

    // SENDER: compute CRC for the given data bits
    static string computeCRC(const string &dataBits) {
        string appended = dataBits + string(GENERATOR.size() - 1, '0');
        return mod2Divide(appended, GENERATOR);
    }

    // SENDER: build the full frame = data + CRC
    static string generateFrame(const string &dataBits) {
        return dataBits + computeCRC(dataBits);
    }

    // RECEIVER: true if an error is detected (nonzero remainder)
    static bool checkForError(const string &receivedFrame) {
        string remainder = mod2Divide(receivedFrame, GENERATOR);
        for (char c : remainder)
            if (c == '1') return true;
        return false;
    }
};

// Simulates a noisy channel: each bit is flipped independently with
// probability equal to the bit error rate (ber). Returns the corrupted
// frame and the number of bits actually flipped.
string passThroughNoisyChannel(const string &frame, double ber, mt19937 &rng, int &flippedCount) {
    string corrupted = frame;
    uniform_real_distribution<double> dist(0.0, 1.0);
    flippedCount = 0;

    for (size_t i = 0; i < corrupted.size(); i++) {
        if (dist(rng) < ber) {
            corrupted[i] = (corrupted[i] == '0') ? '1' : '0';
            flippedCount++;
        }
    }
    return corrupted;
}

void printSeparator() {
    cout << "----------------------------------------------------------\n";
}

int main() {
    cout << "===== CRC over a Noisy Channel - Simulation =====\n";
    cout << "Generator Polynomial : x^16 + x^12 + x^5 + 1\n";
    cout << "Binary Form           : " << GENERATOR << "\n";
    printSeparator();

    string dataBits = "110101101101011010";
    cout << "Original Data Bits : " << dataBits << "\n";

    string frame = CRC::generateFrame(dataBits);
    cout << "Transmitted Frame (Data + CRC) : " << frame << "\n";
    cout << "Frame Length : " << frame.size() << " bits\n";
    printSeparator();

    // Bit error rate for the noisy channel (probability each bit gets flipped)
    double bitErrorRate = 0.05; // 5% chance per bit
    int numTrials = 20;         // number of times we send the frame over the channel

    cout << "Simulating " << numTrials << " transmissions over a noisy channel\n";
    cout << "Bit Error Rate (BER) = " << bitErrorRate << " (per-bit flip probability)\n";
    printSeparator();

    mt19937 rng(random_device{}());
    int correctlyDetected = 0;
    int undetectedErrors = 0;
    int cleanTransmissions = 0;

    cout << left << setw(8) << "Trial"
         << setw(12) << "BitsFlipped"
         << setw(16) << "CRC Result"
         << "Received Frame\n";
    printSeparator();

    for (int trial = 1; trial <= numTrials; trial++) {
        int flipped = 0;
        string received = passThroughNoisyChannel(frame, bitErrorRate, rng, flipped);
        bool errorDetected = CRC::checkForError(received);

        string outcome;
        if (flipped == 0) {
            outcome = "No corruption";
            cleanTransmissions++;
        } else if (errorDetected) {
            outcome = "Error caught";
            correctlyDetected++;
        } else {
            outcome = "MISSED ERROR";
            undetectedErrors++;
        }

        cout << left << setw(8) << trial
             << setw(12) << flipped
             << setw(16) << outcome
             << received << "\n";
    }

    printSeparator();
    cout << "Summary over " << numTrials << " trials:\n";
    cout << "  Clean transmissions (no bit flips)   : " << cleanTransmissions << "\n";
    cout << "  Corrupted & correctly detected by CRC : " << correctlyDetected << "\n";
    cout << "  Corrupted but UNDETECTED by CRC       : " << undetectedErrors << "\n";
    cout << "\nNote: An undetected error happens only when the pattern of bit flips\n";
    cout << "forms a multiple of the generator polynomial - a rare event for a\n";
    cout << "well-chosen generator, which is why CRC is considered highly reliable\n";
    cout << "for detecting random noise-induced errors.\n";
    printSeparator();

    // Interactive mode
    cout << "\nWould you like to test your own data and bit error rate? (y/n): ";
    char choice;
    cin >> choice;

    if (choice == 'y' || choice == 'Y') {
        string userData;
        double userBER;
        int userTrials;

        cout << "Enter binary data (0s and 1s): ";
        cin >> userData;
        cout << "Enter bit error rate (e.g. 0.02 for 2%): ";
        cin >> userBER;
        cout << "Enter number of trials: ";
        cin >> userTrials;

        bool valid = !userData.empty() &&
                     all_of(userData.begin(), userData.end(), [](char c) { return c == '0' || c == '1'; });

        if (!valid) {
            cout << "Invalid binary data entered. Skipping custom simulation.\n";
        } else {
            string userFrame = CRC::generateFrame(userData);
            cout << "\nTransmitted Frame: " << userFrame << "\n";
            printSeparator();

            int det = 0, missed = 0, clean = 0;
            for (int t = 1; t <= userTrials; t++) {
                int flipped = 0;
                string received = passThroughNoisyChannel(userFrame, userBER, rng, flipped);
                bool errorDetected = CRC::checkForError(received);

                string outcome = (flipped == 0) ? "No corruption" :
                                  (errorDetected ? "Error caught" : "MISSED ERROR");
                if (flipped == 0) clean++;
                else if (errorDetected) det++;
                else missed++;

                cout << "Trial " << t << ": flipped=" << flipped
                     << "  outcome=" << outcome << "  frame=" << received << "\n";
            }

            printSeparator();
            cout << "Clean: " << clean << "  Detected: " << det << "  Missed: " << missed << "\n";
        }
    }

    return 0;
}
