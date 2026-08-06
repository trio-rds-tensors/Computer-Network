/*
    Dijkstra's Shortest Path Routing in a Network Graph
    -------------------------------------------------------------
    This program implements Dijkstra's algorithm using an adjacency
    matrix representation (the classic textbook style), commonly used
    to model router/network topologies where each entry cost[i][j]
    represents the link cost between router i and router j (0/INF if
    no direct link exists).

    Features:
      1. Takes the network topology as an adjacency matrix (link costs)
      2. Computes the shortest (least-cost) path from a source router
         to every other router in the network
      3. Displays the complete routing table: Destination, Cost, Path
      4. Also displays the algorithm's internal working - which node
         gets finalized at each step (like link-state routing convergence)

    Compile:  g++ -std=c++17 -O2 dijkstra_network.cpp -o dijkstra_net
    Run:      ./dijkstra_net
*/

#include <bits/stdc++.h>
using namespace std;

const int INF = INT_MAX;

class NetworkGraph {
private:
    int numRouters;
    vector<string> routerName;
    vector<vector<int>> cost; // cost[i][j] = link cost between router i and j (INF if no link)

public:
    NetworkGraph(int n, vector<string> names) {
        numRouters = n;
        routerName = names;
        cost.assign(n, vector<int>(n, INF));
        for (int i = 0; i < n; i++) cost[i][i] = 0;
    }

    void addLink(int u, int v, int linkCost) {
        cost[u][v] = linkCost;
        cost[v][u] = linkCost; // bidirectional link
    }

    void printTopology() {
        cout << "\nNetwork Topology (Link Cost Matrix):\n";
        cout << setw(8) << " ";
        for (int j = 0; j < numRouters; j++) cout << setw(8) << routerName[j];
        cout << "\n";

        for (int i = 0; i < numRouters; i++) {
            cout << setw(8) << routerName[i];
            for (int j = 0; j < numRouters; j++) {
                if (cost[i][j] == INF) cout << setw(8) << "INF";
                else cout << setw(8) << cost[i][j];
            }
            cout << "\n";
        }
    }

    void dijkstra(int src) {
        vector<int> dist(numRouters, INF);
        vector<int> parent(numRouters, -1);
        vector<bool> finalized(numRouters, false);

        dist[src] = 0;

        cout << "\n===== Running Dijkstra's Algorithm from Router "
             << routerName[src] << " =====\n";

        for (int count = 0; count < numRouters; count++) {
            // Pick the un-finalized node with the smallest tentative distance
            int u = -1;
            int minDist = INF;
            for (int v = 0; v < numRouters; v++) {
                if (!finalized[v] && dist[v] < minDist) {
                    minDist = dist[v];
                    u = v;
                }
            }

            if (u == -1) break; // remaining nodes are unreachable

            finalized[u] = true;
            cout << "Step " << (count + 1) << ": Finalized router "
                 << routerName[u] << " with least cost " << dist[u] << "\n";

            // Relax all edges from u
            for (int v = 0; v < numRouters; v++) {
                if (!finalized[v] && cost[u][v] != INF && dist[u] != INF &&
                    dist[u] + cost[u][v] < dist[v]) {
                    dist[v] = dist[u] + cost[u][v];
                    parent[v] = u;
                }
            }
        }

        printRoutingTable(src, dist, parent);
    }

private:
    string reconstructPath(int src, int dest, const vector<int> &parent) {
        if (dest != src && parent[dest] == -1) return "No path";

        vector<int> path;
        for (int at = dest; at != -1; at = parent[at]) path.push_back(at);
        reverse(path.begin(), path.end());

        string result;
        for (size_t i = 0; i < path.size(); i++) {
            result += routerName[path[i]];
            if (i != path.size() - 1) result += " -> ";
        }
        return result;
    }

    void printRoutingTable(int src, const vector<int> &dist, const vector<int> &parent) {
        cout << "\n----------------------------------------------------------\n";
        cout << "Routing Table for Router " << routerName[src] << ":\n";
        cout << "----------------------------------------------------------\n";
        cout << left << setw(14) << "Destination"
             << setw(10) << "Cost"
             << setw(12) << "Next Hop"
             << "Shortest Path\n";
        cout << "----------------------------------------------------------\n";

        for (int dest = 0; dest < numRouters; dest++) {
            if (dest == src) continue;

            cout << left << setw(14) << routerName[dest];

            if (dist[dest] == INF) {
                cout << setw(10) << "INF" << setw(12) << "-" << "Unreachable\n";
                continue;
            }

            cout << setw(10) << dist[dest];

            // Determine next hop: walk back from dest to the node just after src
            int nextHop = dest;
            while (parent[nextHop] != src && parent[nextHop] != -1)
                nextHop = parent[nextHop];

            cout << setw(12) << routerName[nextHop];
            cout << reconstructPath(src, dest, parent) << "\n";
        }
        cout << "----------------------------------------------------------\n";
    }
};

int main() {
    // ------------------------------------------------------
    // Example network topology (edit as needed)
    //
    //          7         9
    //     A ------- B ------- C
    //     |  \                |
    //     14  \10             2
    //     |    \               |
    //     F ---- E ------- D --+
    //       2         15    6  (D-C link cost 6 not shown above, see edges below)
    //
    // Classic 6-router topology similar to well-known Dijkstra textbook example
    // ------------------------------------------------------

    vector<string> names = {"A", "B", "C", "D", "E", "F"};
    NetworkGraph network(names.size(), names);

    network.addLink(0, 1, 7);   // A - B
    network.addLink(0, 2, 9);   // A - C
    network.addLink(0, 5, 14);  // A - F
    network.addLink(1, 2, 10);  // B - C
    network.addLink(1, 3, 15);  // B - D
    network.addLink(2, 3, 11);  // C - D
    network.addLink(2, 5, 2);   // C - F
    network.addLink(3, 4, 6);   // D - E
    network.addLink(4, 5, 9);   // E - F

    network.printTopology();

    // Run Dijkstra's algorithm from router A
    network.dijkstra(0);

    // Run again from a different source to show another router's table
    network.dijkstra(3); // from router D

    return 0;
}
