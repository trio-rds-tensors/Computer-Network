/*
    Distance Vector Routing (DVR) Algorithm Simulation
    ----------------------------------------------------
    - Bellman-Ford ভিত্তিক Distance Vector Routing অ্যালগরিদম সিমুলেশন
    - প্রতিটি নোড তার প্রতিবেশীদের (neighbors) সাথে distance vector শেয়ার করে
    - প্রতি iteration-এ routing table আপডেট হয়, convergence না হওয়া পর্যন্त চলতে থাকে
    - INF = কোনো সরাসরি সংযোগ নেই বোঝাতে ব্যবহৃত হয়েছে

    Compile:  g++ -std=c++17 -O2 distance_vector_routing.cpp -o dvr
    Run:      ./dvr
*/

#include <bits/stdc++.h>
using namespace std;

const int INF = 999999; // Infinity বোঝাতে বড় সংখ্যা ব্যবহার করা হয়েছে

class DistanceVectorRouting {
private:
    int n;                              // মোট নোড সংখ্যা
    vector<string> nodeNames;           // নোডের নাম (A, B, C, ...)
    vector<vector<int>> dist;           // dist[i][j] = i থেকে j পর্যন্ত সর্বনিম্ন cost
    vector<vector<int>> nextHop;        // nextHop[i][j] = i থেকে j যেতে হলে প্রথম কোন নোডে যেতে হবে

public:
    DistanceVectorRouting(int numNodes, vector<string> names) {
        n = numNodes;
        nodeNames = names;
        dist.assign(n, vector<int>(n, INF));
        nextHop.assign(n, vector<int>(n, -1));

        for (int i = 0; i < n; i++) {
            dist[i][i] = 0;
            nextHop[i][i] = i;
        }
    }

    // সরাসরি সংযোগ (direct link) যোগ করার ফাংশন
    void addEdge(int u, int v, int cost) {
        dist[u][v] = cost;
        dist[v][u] = cost;
        nextHop[u][v] = v;
        nextHop[v][u] = u;
    }

    // বর্তমান routing table প্রিন্ট করার ফাংশন
    void printRoutingTables(int iteration) {
        cout << "\n================ Iteration " << iteration << " ================\n";
        for (int i = 0; i < n; i++) {
            cout << "\nRouting Table for Node " << nodeNames[i] << ":\n";
            cout << left << setw(12) << "Destination"
                 << setw(10) << "Cost"
                 << setw(12) << "Next Hop" << "\n";
            cout << "----------------------------------------\n";
            for (int j = 0; j < n; j++) {
                cout << left << setw(12) << nodeNames[j];
                if (dist[i][j] >= INF)
                    cout << setw(10) << "INF";
                else
                    cout << setw(10) << dist[i][j];

                if (nextHop[i][j] == -1)
                    cout << setw(12) << "-";
                else
                    cout << setw(12) << nodeNames[nextHop[i][j]];
                cout << "\n";
            }
        }
    }

    // মূল Distance Vector Routing অ্যালগরিদম (Bellman-Ford ভিত্তিক)
    void runSimulation() {
        cout << "Starting Distance Vector Routing Simulation...\n";
        bool updated = true;
        int iteration = 0;

        printRoutingTables(iteration); // Initial state

        while (updated) {
            updated = false;
            iteration++;

            // প্রতিটি নোড তার প্রতিবেশীর distance vector থেকে তথ্য নিয়ে
            // নিজের routing table আপডেট করার চেষ্টা করবে
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    if (i == j) continue;

                    // i-এর প্রতিটি প্রতিবেশী k-এর মাধ্যমে j-তে যাওয়ার cost check করা হচ্ছে
                    for (int k = 0; k < n; k++) {
                        if (k == i) continue;
                        if (dist[i][k] >= INF) continue; // k প্রতিবেশী নয়
                        if (dist[k][j] >= INF) continue; // k থেকে j-এর route নেই

                        int newCost = dist[i][k] + dist[k][j];
                        if (newCost < dist[i][j]) {
                            dist[i][j] = newCost;
                            nextHop[i][j] = k;
                            updated = true; // পরিবর্তন হয়েছে, তাই আরেকবার loop চালাতে হবে
                        }
                    }
                }
            }

            if (updated) {
                printRoutingTables(iteration);
            }
        }

        cout << "\n✅ Convergence reached after " << iteration << " iteration(s).\n";
        printRoutingTables(iteration);
    }

    // Link cost পরিবর্তন হলে (link failure বা নতুন link) পুনরায় সিমুলেশন চালানোর ফাংশন
    void changeLinkCost(int u, int v, int newCost) {
        cout << "\n\n*** Link cost changed between " << nodeNames[u]
             << " and " << nodeNames[v] << " -> new cost = "
             << (newCost >= INF ? "INF (link down)" : to_string(newCost)) << " ***\n";
        dist[u][v] = newCost;
        dist[v][u] = newCost;
        if (newCost >= INF) {
            nextHop[u][v] = -1;
            nextHop[v][u] = -1;
        } else {
            nextHop[u][v] = v;
            nextHop[v][u] = u;
        }
        runSimulation();
    }
};

int main() {
    // ------------------------------------------------------
    // উদাহরণ নেটওয়ার্ক টপোলজি (এডিট করে নিজের মতো বানাতে পারবেন)
    //
    //        A ---1--- B
    //        |         |
    //        4         2
    //        |         |
    //        C ---5--- D
    //
    // ------------------------------------------------------

    vector<string> names = {"A", "B", "C", "D"};
    DistanceVectorRouting network(names.size(), names);

    network.addEdge(0, 1, 1);  // A - B, cost 1
    network.addEdge(0, 2, 4);  // A - C, cost 4
    network.addEdge(1, 3, 2);  // B - D, cost 2
    network.addEdge(2, 3, 5);  // C - D, cost 5

    // প্রাথমিক সিমুলেশন চালানো (convergence পর্যন্ত)
    network.runSimulation();

    // ঐচ্ছিক: একটি link cost পরিবর্তন করে দেখানো (উদাহরণ হিসেবে)
    // যেমন B-D লিংকের cost 2 থেকে বেড়ে 10 হলো
    network.changeLinkCost(1, 3, 10);

    return 0;
}
