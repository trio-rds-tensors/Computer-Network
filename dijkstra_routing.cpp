/*
 * Dijkstra's Shortest Path Algorithm - Network Routing Simulation
 * -------------------------------------------------------------------
 * Models a network as a weighted, undirected graph where:
 *   - Nodes  = routers / hosts
 *   - Edges  = links between routers
 *   - Weight = link cost (e.g. delay, hop metric, bandwidth cost)
 *
 * Dijkstra's algorithm computes the minimum-cost path from a source
 * router to every other router in the network - exactly the kind of
 * computation link-state routing protocols (like OSPF) perform to
 * build their routing tables.
 *
 * Implementation uses an adjacency list + a min-priority-queue
 * (O((V+E) log V)) and reconstructs the actual path for each
 * destination, not just the distance.
 */

#include <iostream>
#include <vector>
#include <queue>
#include <climits>
#include <iomanip>
#include <string>
#include <algorithm>
using namespace std;

const int INF = INT_MAX;

struct Edge {
    int to;
    int weight;
};

class NetworkGraph {
private:
    int numNodes;
    vector<vector<Edge>> adj;
    vector<string> nodeNames;

public:
    NetworkGraph(int n) : numNodes(n), adj(n), nodeNames(n) {
        for (int i = 0; i < n; i++) nodeNames[i] = "R" + to_string(i);
    }

    void setNodeName(int id, const string &name) {
        if (id >= 0 && id < numNodes) nodeNames[id] = name;
    }

    string name(int id) const { return nodeNames[id]; }

    // Add a bidirectional link between two routers with a given cost
    void addLink(int u, int v, int cost) {
        adj[u].push_back({v, cost});
        adj[v].push_back({u, cost});
    }

    // Run Dijkstra's algorithm from 'source'.
    // Fills dist[] with shortest cost to every node, and parent[]
    // to allow path reconstruction.
    void dijkstra(int source, vector<int> &dist, vector<int> &parent) const {
        dist.assign(numNodes, INF);
        parent.assign(numNodes, -1);
        vector<bool> visited(numNodes, false);

        // min-heap of (distance, node)
        priority_queue<pair<int,int>, vector<pair<int,int>>, greater<>> pq;

        dist[source] = 0;
        pq.push({0, source});

        cout << "\n--- Dijkstra trace from source " << nodeNames[source] << " ---\n";

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (visited[u]) continue;   // stale entry, skip
            visited[u] = true;

            cout << "Finalized " << nodeNames[u]
                 << " with shortest distance = " << d << "\n";

            for (const Edge &e : adj[u]) {
                if (visited[e.to]) continue;
                int newDist = dist[u] + e.weight;
                if (newDist < dist[e.to]) {
                    dist[e.to] = newDist;
                    parent[e.to] = u;
                    pq.push({newDist, e.to});
                    cout << "  Relax edge " << nodeNames[u] << " -> " << nodeNames[e.to]
                         << " (cost " << e.weight << "): new best dist = " << newDist << "\n";
                }
            }
        }
    }

    // Reconstruct path from source to dest using the parent array
    vector<int> reconstructPath(int source, int dest, const vector<int> &parent) const {
        vector<int> path;
        if (parent[dest] == -1 && dest != source) return path; // unreachable

        for (int at = dest; at != -1; at = parent[at]) {
            path.push_back(at);
            if (at == source) break;
        }
        reverse(path.begin(), path.end());
        return path;
    }

    void printRoutingTable(int source, const vector<int> &dist, const vector<int> &parent) const {
        cout << "\n================ Routing Table for " << nodeNames[source]
             << " ================\n";
        cout << left << setw(12) << "Destination"
             << setw(12) << "Cost"
             << "Path\n";

        for (int v = 0; v < numNodes; v++) {
            if (v == source) continue;
            cout << left << setw(12) << nodeNames[v];
            if (dist[v] == INF) {
                cout << setw(12) << "INF" << "unreachable\n";
                continue;
            }
            cout << setw(12) << dist[v];

            vector<int> path = reconstructPath(source, v, parent);
            for (size_t i = 0; i < path.size(); i++) {
                cout << nodeNames[path[i]];
                if (i + 1 < path.size()) cout << " -> ";
            }
            cout << "\n";
        }
        cout << "========================================================\n";
    }
};

int main() {
    // ---------------------------------------------------------------
    // Build a sample network topology (routers R0..R5)
    //
    //        2         3
    //   R0 ------- R1 ------- R2
    //   |  \        |          |
    //  4|   7\      1          5
    //   |      \    |          |
    //   R3 ------- R4 ------- R5
    //        2           1
    // ---------------------------------------------------------------
    int n = 6;
    NetworkGraph net(n);

    net.setNodeName(0, "R0");
    net.setNodeName(1, "R1");
    net.setNodeName(2, "R2");
    net.setNodeName(3, "R3");
    net.setNodeName(4, "R4");
    net.setNodeName(5, "R5");

    net.addLink(0, 1, 2);   // R0 - R1
    net.addLink(1, 2, 3);   // R1 - R2
    net.addLink(0, 3, 4);   // R0 - R3
    net.addLink(0, 4, 7);   // R0 - R4
    net.addLink(1, 4, 1);   // R1 - R4
    net.addLink(2, 5, 5);   // R2 - R5
    net.addLink(3, 4, 2);   // R3 - R4
    net.addLink(4, 5, 1);   // R4 - R5

    cout << "==================================================\n";
    cout << "   Dijkstra's Algorithm - Network Routing Demo\n";
    cout << "==================================================\n";
    cout << "Nodes: R0, R1, R2, R3, R4, R5\n";
    cout << "Links (cost):\n";
    cout << "  R0-R1(2)  R1-R2(3)  R0-R3(4)  R0-R4(7)\n";
    cout << "  R1-R4(1)  R2-R5(5)  R3-R4(2)  R4-R5(1)\n";

    int source = 0;
    vector<int> dist, parent;
    net.dijkstra(source, dist, parent);
    net.printRoutingTable(source, dist, parent);

    // Demonstrate from a second source too, to show reusability
    source = 2;
    net.dijkstra(source, dist, parent);
    net.printRoutingTable(source, dist, parent);

    return 0;
}
