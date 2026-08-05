// Distance Vector Routing Simulation (Bellman-Ford based)
// ---------------------------------------------------------
// Each node keeps a distance vector: cost to reach every other node.
// Nodes exchange their vectors with direct neighbors only.
// On receiving a neighbor's vector, a node updates its own distances using:
//     dist[me][dest] = min( dist[me][dest], cost(me, neighbor) + dist[neighbor][dest] )
// This repeats until no distance changes (convergence) -> shortest paths found.
// Also tracks "next hop" so we can print the actual optimal path.

#include <iostream>
#include <vector>
#include <climits>
#include <string>

using namespace std;

const int INF = 100000; // represents "no known path"

int main() {
    int n;
    cout << "Enter number of nodes: ";
    cin >> n;

    vector<string> name(n);
    cout << "Enter node names (e.g. A B C D):\n";
    for (int i = 0; i < n; i++) cin >> name[i];

    // cost[i][j] = direct link cost between i and j (INF if no direct link)
    vector<vector<int>> cost(n, vector<int>(n, INF));
    for (int i = 0; i < n; i++) cost[i][i] = 0;

    cout << "Enter number of direct links: ";
    int links;
    cin >> links;

    cout << "For each link enter: node1 node2 cost\n";
    for (int k = 0; k < links; k++) {
        string a, b;
        int c;
        cin >> a >> b >> c;
        int i = -1, j = -1;
        for (int x = 0; x < n; x++) {
            if (name[x] == a) i = x;
            if (name[x] == b) j = x;
        }
        if (i == -1 || j == -1) {
            cout << "  Unknown node name, skipping.\n";
            continue;
        }
        cost[i][j] = c;
        cost[j][i] = c; // assume bidirectional link
    }

    // dist[i][j] = current best known cost from node i to node j
    // nextHop[i][j] = which neighbor node i should forward to, to reach j
    vector<vector<int>> dist = cost;
    vector<vector<int>> nextHop(n, vector<int>(n, -1));
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            if (i != j && cost[i][j] < INF) nextHop[i][j] = j;

    // Distance Vector algorithm: repeatedly relax using neighbor vectors
    // until nothing changes (this simulates rounds of vector exchange).
    bool updated = true;
    int round = 0;
    while (updated) {
        updated = false;
        round++;
        // Node i learns from each direct neighbor k
        for (int i = 0; i < n; i++) {
            for (int k = 0; k < n; k++) {
                if (i == k || cost[i][k] >= INF) continue; // k must be a direct neighbor
                for (int j = 0; j < n; j++) {
                    if (dist[k][j] >= INF) continue;
                    int viaK = cost[i][k] + dist[k][j];
                    if (viaK < dist[i][j]) {
                        dist[i][j] = viaK;
                        nextHop[i][j] = nextHop[i][k]; // forward towards k first
                        updated = true;
                    }
                }
            }
        }
    }
    cout << "\nConverged after " << round << " rounds.\n";

    // Print final routing table for each node
    for (int i = 0; i < n; i++) {
        cout << "\nRouting table for node " << name[i] << ":\n";
        cout << "  Dest\tCost\tNext Hop\n";
        for (int j = 0; j < n; j++) {
            if (i == j) continue;
            cout << "  " << name[j] << "\t";
            if (dist[i][j] >= INF) {
                cout << "INF\tunreachable\n";
            } else {
                cout << dist[i][j] << "\t" << name[nextHop[i][j]] << "\n";
            }
        }
    }

    // Let user query full path between any two nodes
    cout << "\nEnter source and destination to see full path (or 'q q' to quit):\n";
    string s, d;
    while (cin >> s >> d) {
        if (s == "q" || d == "q") break;
        int si = -1, di = -1;
        for (int x = 0; x < n; x++) {
            if (name[x] == s) si = x;
            if (name[x] == d) di = x;
        }
        if (si == -1 || di == -1) {
            cout << "Unknown node.\n";
            continue;
        }
        if (dist[si][di] >= INF) {
            cout << "No path from " << s << " to " << d << "\n";
        } else {
            cout << "Path: " << s;
            int cur = si;
            while (cur != di) {
                cur = nextHop[cur][di];
                cout << " -> " << name[cur];
            }
            cout << "  (total cost = " << dist[si][di] << ")\n";
        }
        cout << "\nEnter source and destination (or 'q q' to quit):\n";
    }

    return 0;
}
