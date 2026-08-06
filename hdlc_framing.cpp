/*
 * HDLC (High-Level Data Link Control) Framing
 * ---------------------------------------------
 * Implements:
 *   1. Bit Stuffing   - operates on a string of '0'/'1' bits.
 *                        Flag = 01111110. After 5 consecutive 1s in the
 *                        data, a 0 is stuffed to avoid the data being
 *                        mistaken for a flag.
 *   2. Character Stuffing - operates on a string of bytes/characters.
 *                        FLAG = 0x7E, ESC = 0x7D.
 *                        Any occurrence of FLAG or ESC in the data is
 *                        preceded by an ESC byte.
 *
 * Both stuffing and destuffing (reverse process) are shown so the
 * frame can be verified by recovering the original data.
 */

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

const string BIT_FLAG = "01111110";     // HDLC flag pattern (bit stuffing)
const unsigned char CHAR_FLAG = 0x7E;   // Flag byte (character stuffing)
const unsigned char CHAR_ESC  = 0x7D;   // Escape byte (character stuffing)

/* ----------------------------------------------------------------
 * 1. BIT STUFFING
 * ---------------------------------------------------------------- */

// Insert a '0' after every five consecutive '1's in the data bits
string bitStuff(const string &data) {
    string stuffed;
    int oneCount = 0;

    for (char bit : data) {
        stuffed += bit;
        if (bit == '1') {
            oneCount++;
            if (oneCount == 5) {
                stuffed += '0';   // stuff a 0
                oneCount = 0;
            }
        } else {
            oneCount = 0;
        }
    }
    return stuffed;
}

// Remove the stuffed '0' that follows five consecutive '1's
string bitDestuff(const string &stuffed) {
    string data;
    int oneCount = 0;

    for (size_t i = 0; i < stuffed.size(); i++) {
        char bit = stuffed[i];

        if (oneCount == 5) {
            // this bit must be the stuffed 0 -> discard it, don't add
            oneCount = 0;
            continue;
        }

        data += bit;
        if (bit == '1') oneCount++;
        else oneCount = 0;
    }
    return data;
}

// Build a full HDLC frame: FLAG + stuffed data + FLAG (bit level)
string buildBitFrame(const string &data) {
    return BIT_FLAG + bitStuff(data) + BIT_FLAG;
}

/* ----------------------------------------------------------------
 * 2. CHARACTER STUFFING
 * ---------------------------------------------------------------- */

// Insert ESC before every FLAG or ESC byte found in the data
vector<unsigned char> charStuff(const vector<unsigned char> &data) {
    vector<unsigned char> stuffed;

    for (unsigned char ch : data) {
        if (ch == CHAR_FLAG || ch == CHAR_ESC) {
            stuffed.push_back(CHAR_ESC);
        }
        stuffed.push_back(ch);
    }
    return stuffed;
}

// Remove the ESC bytes inserted during stuffing
vector<unsigned char> charDestuff(const vector<unsigned char> &stuffed) {
    vector<unsigned char> data;

    for (size_t i = 0; i < stuffed.size(); i++) {
        if (stuffed[i] == CHAR_ESC && i + 1 < stuffed.size()) {
            data.push_back(stuffed[i + 1]);
            i++;   // skip the escaped byte
        } else {
            data.push_back(stuffed[i]);
        }
    }
    return data;
}

// Build a full HDLC frame: FLAG + stuffed data + FLAG (character level)
vector<unsigned char> buildCharFrame(const vector<unsigned char> &data) {
    vector<unsigned char> frame;
    frame.push_back(CHAR_FLAG);
    vector<unsigned char> stuffed = charStuff(data);
    frame.insert(frame.end(), stuffed.begin(), stuffed.end());
    frame.push_back(CHAR_FLAG);
    return frame;
}

/* ----------------------------------------------------------------
 * Helpers to print results
 * ---------------------------------------------------------------- */

void printCharVector(const vector<unsigned char> &v) {
    for (unsigned char c : v) {
        cout << "0x" << hex << uppercase << setw(2) << setfill('0')
             << (int)c << " ";
    }
    cout << dec << endl;
}

string vectorToString(const vector<unsigned char> &v) {
    string s;
    for (unsigned char c : v) s += (char)c;
    return s;
}

/* ----------------------------------------------------------------
 * MAIN - demonstration
 * ---------------------------------------------------------------- */

int main() {
    cout << "=====================================================\n";
    cout << "        HDLC FRAMING: BIT STUFFING DEMO\n";
    cout << "=====================================================\n";

    string dataBits;
    cout << "Enter binary data (0/1 only), e.g. 110111110101111110:\n> ";
    cin >> dataBits;

    string stuffedBits = bitStuff(dataBits);
    string frameBits   = buildBitFrame(dataBits);
    string recoveredBits = bitDestuff(stuffedBits);

    cout << "\nOriginal Data     : " << dataBits << endl;
    cout << "After Bit Stuffing: " << stuffedBits << endl;
    cout << "Framed (FLAG+data+FLAG): " << frameBits << endl;
    cout << "Destuffed (recovered)  : " << recoveredBits << endl;
    cout << "Recovery " << (recoveredBits == dataBits ? "SUCCESS" : "FAILED")
         << endl;

    cout << "\n=====================================================\n";
    cout << "        HDLC FRAMING: CHARACTER STUFFING DEMO\n";
    cout << "=====================================================\n";

    cin.ignore();
    string dataChars;
    cout << "Enter text data (may contain '~' as FLAG, '}' as ESC\n"
         << "for testing, since 0x7E='~' and 0x7D='}'):\n> ";
    getline(cin, dataChars);

    vector<unsigned char> dataBytes(dataChars.begin(), dataChars.end());
    vector<unsigned char> stuffedBytes = charStuff(dataBytes);
    vector<unsigned char> frameBytes   = buildCharFrame(dataBytes);
    vector<unsigned char> recoveredBytes = charDestuff(stuffedBytes);

    cout << "\nOriginal Data (text) : " << dataChars << endl;
    cout << "Original Data (hex)  : ";
    printCharVector(dataBytes);

    cout << "After Char Stuffing  : ";
    printCharVector(stuffedBytes);

    cout << "Framed (FLAG+data+FLAG), hex: ";
    printCharVector(frameBytes);

    cout << "Destuffed (recovered): " << vectorToString(recoveredBytes) << endl;
    cout << "Recovery "
         << (vectorToString(recoveredBytes) == dataChars ? "SUCCESS" : "FAILED")
         << endl;

    return 0;
}
