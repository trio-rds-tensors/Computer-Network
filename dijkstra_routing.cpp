/*
    Dijkstra's Algorithm - Least-Cost Path Routing Simulation
    -------------------------------------------------------------
    This program simulates link-state routing using Dijkstra's shortest
    (least-cost) path algorithm, commonly used in protocols like OSPF.

    What this program does:
      1. Builds a weighted network graph (nodes = routers, edge weight = link cost)
      2. Runs Dijkstra's algorithm from a source node to find the least-cost
         path to every other node in the network
      3. Prints the routing table: Destination, Least Cost, Next Hop, and
         the full path taken
      4. Uses a min-priority-queue for efficient O((V+E) log V) computation

    Compile:  g++ -std=c++17 -O2 dijkstra_routing.cpp -o dijkstra
    Run:      ./dijkstra
*/

#include <bits/stdc++.h>
using namespace std;

const int INF = 999999;

class DijkstraRouting {
private:
    int n;                                      // number of nodes
    vector<string> nodeNames;                   // node labels
    vector<vector<pair<int,int>>> adj;           // adjacency list: adj[u] = {(v, cost), ...}

public:
    DijkstraRouting(int numNodes, vector<string> names) {
        n = numNodes;
        nodeNames = names;
        adj.assign(n, {});
    }

    // Add a bidirectional link between u and v with the given cost
    void addEdge(int u, int v, int cost) {
        adj[u].push_back({v, cost});
        adj[v].push_back({u, cost});
    }

    // Runs Dijkstra's algorithm from the given source node
    void runDijkstra(int source) {
        vector<int> dist(n, INF);
        vector<int> parent(n, -1);
        vector<bool> visited(n, false);

        // Min-heap of (distance, node)
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<pair<int,int>>> pq;

        dist[source] = 0;
        pq.push({0, source});

        cout << "\n===== Running Dijkstra's Algorithm from Source Node "
             << nodeNames[source] << " =====\n";

        int step = 0;
        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (visited[u]) continue;
            visited[u] = true;
            step++;

            cout << "Step " << step << ": Finalized node " << nodeNames[u]
                 << " with least cost " << d << "\n";

            for (auto &[v, cost] : adj[u]) {
                if (!visited[v] && dist[u] + cost < dist[v]) {
                    dist[v] = dist[u] + cost;
                    parent[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }

        printRoutingTable(source, dist, parent);
    }

    // Reconstructs and prints the path from source to destination
    string getPath(int source, int dest, const vector<int> &parent) {
        if (dest != source && parent[dest] == -1)
            return "No path";

        vector<int> path;
        for (int at = dest; at != -1; at = parent[at])
            path.push_back(at);
        reverse(path.begin(), path.end());

        string result;
        for (size_t i = 0; i < path.size(); i++) {
            result += nodeNames[path[i]];
            if (i != path.size() - 1) result += " -> ";
        }
        return result;
    }

    // Prints the final routing table: Destination, Cost, Next Hop, Full Path
    void printRoutingTable(int source, const vector<int> &dist, const vector<int> &parent) {
        cout << "\n----------------------------------------------------------\n";
        cout << "Routing Table for Source Node " << nodeNames[source] << ":\n";
        cout << "----------------------------------------------------------\n";
        cout << left << setw(14) << "Destination"
             << setw(10) << "Cost"
             << setw(12) << "Next Hop"
             << "Full Path\n";
        cout << "----------------------------------------------------------\n";

        for (int dest = 0; dest < n; dest++) {
            if (dest == source) continue;

            cout << left << setw(14) << nodeNames[dest];

            if (dist[dest] >= INF) {
                cout << setw(10) << "INF" << setw(12) << "-" << "No path\n";
                continue;
            }

            cout << setw(10) << dist[dest];

            // Find next hop: the node right after source on the path
            int nextHop = dest;
            while (parent[nextHop] != source && parent[nextHop] != -1)
                nextHop = parent[nextHop];

            cout << setw(12) << nodeNames[nextHop];
            cout << getPath(source, dest, parent) << "\n";
        }
        cout << "----------------------------------------------------------\n";
    }
};

int main() {
    // ------------------------------------------------------
    // Example network topology (edit as needed)
    //
    //          2         3
    //     A ------- B ------- C
    //     |         |         |
    //     6         1         5
    //     |         |         |
    //     D ------- E ------- F
    //          2         4
    //
    // ------------------------------------------------------

    vector<string> names = {"A", "B", "C", "D", "E", "F"};
    DijkstraRouting network(names.size(), names);

    network.addEdge(0, 1, 2);  // A - B, cost 2
    network.addEdge(1, 2, 3);  // B - C, cost 3
    network.addEdge(0, 3, 6);  // A - D, cost 6
    network.addEdge(1, 4, 1);  // B - E, cost 1
    network.addEdge(2, 5, 5);  // C - F, cost 5
    network.addEdge(3, 4, 2);  // D - E, cost 2
    network.addEdge(4, 5, 4);  // E - F, cost 4

    // Run Dijkstra's algorithm from source node A
    network.runDijkstra(0);

    // Try another source node to show routing from a different router
    network.runDijkstra(3); // from node D

    return 0;
}
