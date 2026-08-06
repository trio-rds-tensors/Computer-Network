/*
    CRC-CCITT Polynomial Computation & Error Checking
    ----------------------------------------------------
    Generator Polynomial : x^16 + x^12 + x^5 + 1   ->  binary: 10001000000100001
    Method                : Standard binary (modulo-2) polynomial division,
                             the same technique taught for CRC generation/checking.

    What this program does:
      1. SENDER SIDE   : Appends 16 zero bits to the data, divides by the
                          generator polynomial (mod-2 / XOR division), and the
                          16-bit remainder becomes the CRC. This CRC is appended
                          to the original data to form the transmitted frame.
      2. RECEIVER SIDE : Divides the received frame (data + CRC) by the same
                          generator polynomial. If the remainder is all zeros,
                          the frame is accepted as error-free; otherwise an
                          error is detected.

    Compile:  g++ -std=c++17 -O2 crc_ccitt.cpp -o crc_ccitt
    Run:      ./crc_ccitt
*/

#include <bits/stdc++.h>
using namespace std;

// CRC-CCITT generator polynomial: x^16 + x^12 + x^5 + 1  (17 bits, degree 16)
const string GENERATOR = "10001000000100001";

class CRC_CCITT {
public:
    // Bitwise XOR of two equal-length binary strings
    static string xorBits(const string &a, const string &b) {
        string result = "";
        for (size_t i = 1; i < a.size(); i++) // skip leading bit (like long division)
            result += (a[i] == b[i]) ? '0' : '1';
        return result;
    }

    // Modulo-2 (XOR) binary division. Returns the remainder.
    // 'dividend' must already include the appended zero bits (for CRC generation)
    // or be the raw received frame (for error checking).
    static string mod2Divide(string dividend, const string &divisor) {
        int genLen = divisor.size();
        string current = dividend.substr(0, genLen);

        for (size_t i = genLen; i < dividend.size(); i++) {
            if (current[0] == '1')
                current = xorBits(divisor, current) + dividend[i];
            else
                current = xorBits(string(genLen, '0'), current) + dividend[i];
        }

        // Final XOR step for the last chunk
        if (current[0] == '1')
            current = xorBits(divisor, current);
        else
            current = xorBits(string(genLen, '0'), current);

        return current; // remainder, length = genLen - 1 = 16 bits for CRC-CCITT
    }

    // SENDER: compute the 16-bit CRC for given data bits
    static string computeCRC(const string &dataBits) {
        string appended = dataBits + string(GENERATOR.size() - 1, '0'); // append 16 zero bits
        return mod2Divide(appended, GENERATOR);
    }

    // SENDER: build the full transmitted frame = data + CRC
    static string generateFrame(const string &dataBits) {
        return dataBits + computeCRC(dataBits);
    }

    // RECEIVER: check the received frame for errors.
    // Returns true if an ERROR is detected (remainder != 0), false if frame is clean.
    static bool checkForError(const string &receivedFrame) {
        string remainder = mod2Divide(receivedFrame, GENERATOR);
        for (char c : remainder)
            if (c == '1') return true; // any 1-bit means an error was detected
        return false;
    }

    static uint32_t toHex(const string &bits) {
        uint32_t value = 0;
        for (char c : bits) value = (value << 1) | (c - '0');
        return value;
    }

    // Flip one bit at the given position to simulate a transmission error
    static string introduceError(string bits, int position) {
        if (position >= 0 && position < (int)bits.size())
            bits[position] = (bits[position] == '0') ? '1' : '0';
        return bits;
    }
};

void printSeparator() {
    cout << "----------------------------------------------------------\n";
}

int main() {
    cout << "===== CRC-CCITT Polynomial Computation & Error Checking =====\n";
    cout << "Generator Polynomial : x^16 + x^12 + x^5 + 1\n";
    cout << "Binary Form           : " << GENERATOR << "\n";
    printSeparator();

    // Example data to send
    string dataBits = "1101011011";
    cout << "Original Data Bits : " << dataBits << "\n";

    // Step 1: Sender computes CRC and builds the frame
    string crc = CRC_CCITT::computeCRC(dataBits);
    cout << "Computed CRC (16-bit) : " << crc
         << "  (0x" << hex << uppercase << CRC_CCITT::toHex(crc) << dec << ")\n";

    string frame = CRC_CCITT::generateFrame(dataBits);
    cout << "Transmitted Frame (Data + CRC) : " << frame << "\n";
    printSeparator();

    // Step 2: Receiver checks the frame WITHOUT any error
    cout << "[Case 1] Receiver gets the frame with NO transmission error:\n";
    bool err1 = CRC_CCITT::checkForError(frame);
    cout << "Result: " << (err1 ? "Error detected!" : "No error detected. Frame accepted.") << "\n";
    printSeparator();

    // Step 3: Simulate a transmission error by flipping a bit
    int errorPos = 3; // flip bit at index 3 (0-indexed)
    string corruptedFrame = CRC_CCITT::introduceError(frame, errorPos);
    cout << "[Case 2] Receiver gets the frame WITH a transmission error (bit "
         << errorPos << " flipped):\n";
    cout << "Corrupted Frame : " << corruptedFrame << "\n";

    bool err2 = CRC_CCITT::checkForError(corruptedFrame);
    cout << "Result: " << (err2 ? "Error detected! Frame rejected." : "No error detected.") << "\n";
    printSeparator();

    // Interactive mode: let the user try their own data
    cout << "\nEnter your own binary data (0s and 1s), or press Enter to skip: ";
    string userInput;
    getline(cin, userInput);

    if (!userInput.empty() &&
        all_of(userInput.begin(), userInput.end(), [](char c) { return c == '0' || c == '1'; })) {

        string userCRC = CRC_CCITT::computeCRC(userInput);
        string userFrame = CRC_CCITT::generateFrame(userInput);

        cout << "\nYour Data     : " << userInput << "\n";
        cout << "Computed CRC  : " << userCRC
             << " (0x" << hex << uppercase << CRC_CCITT::toHex(userCRC) << dec << ")\n";
        cout << "Frame to Send : " << userFrame << "\n";

        bool ok = CRC_CCITT::checkForError(userFrame);
        cout << "Receiver Check: " << (ok ? "Error detected!" : "Frame OK, no error.") << "\n";
    }

    return 0;
}
