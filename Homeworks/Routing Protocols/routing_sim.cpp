#include <iostream>
#include <vector>
#include <limits>
#include <queue>
#include <fstream>
#include <sstream>
#include <iomanip>

using namespace std;

const int INF = 9999;

void printDVRTable(int node, const vector<vector<int>>& table, const vector<vector<int>>& nextHop) {
    cout << "Node " << node << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < table.size(); ++i) {
        cout << i << "\t" << table[node][i] << "\t";
        if (nextHop[node][i] == -1) cout << "-";
        else cout << nextHop[node][i];
        cout << endl;
    }
    cout << endl;
}

void simulateDVR(const vector<vector<int>>& graph) {
    int n = graph.size();
    vector<vector<int>> dist = graph;
    vector<vector<int>> nextHop(n, vector<int>(n));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            dist[i][j] = graph[i][j];
            if (i != j) {
                if (graph[i][j] != INF) {
                    nextHop[i][j] = j;
                } else {
                    nextHop[i][j] = -1;
                }
            } else {
                dist[i][j] = 0;
                nextHop[i][j] = -1;
            }
        }
    }
    //exchanging the routing tables and updating the tables 
    bool updated;
    do {
        updated = false;
        vector<vector<int>> prev_dist = dist;

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                if (i == j) continue;

                int min_cost = prev_dist[i][j];
                int min_hop = nextHop[i][j];

                for (int k = 0; k < n; ++k) {
                    if (k == i || prev_dist[i][k] == INF) continue;

                    int possible_cost = prev_dist[i][k] + prev_dist[k][j];
                    if (possible_cost < min_cost) {
                        min_cost = possible_cost;
                        min_hop = k;
                    }
                }

                if (min_cost < dist[i][j]) {
                    dist[i][j] = min_cost;
                    nextHop[i][j] = min_hop;
                    updated = true;
                }
            }
        }
    } while (updated);

    cout << "--- DVR Final Tables ---\n";
    for (int i = 0; i < n; ++i) printDVRTable(i, dist, nextHop);
}

void printLSRTable(int src, const vector<int>& dist, const vector<int>& prev) {
    cout << "Node " << src << " Routing Table:\n";
    cout << "Dest\tCost\tNext Hop\n";
    for (int i = 0; i < dist.size(); ++i) {
        if (i == src) continue;
        cout << i << "\t" << dist[i] << "\t";
        int hop = i;
        while (prev[hop] != src && prev[hop] != -1)
            hop = prev[hop];
        cout << (prev[hop] == -1 ? -1 : hop) << endl;
    }
    cout << endl;
}

void simulateLSR(const vector<vector<int>>& graph) {
    int n = graph.size();
    for (int src = 0; src < n; ++src) {
        vector<int> dist(n, INF);
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);
        dist[src] = 0;
        
        //applying dijakstra's algorithm
        priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;
        pq.push({0, src});//intializing priority queque for distance

        while (!pq.empty()) {
            int u = pq.top().second;
            int current_dist = pq.top().first;
            pq.pop();

            if (visited[u]) continue;
            visited[u] = true;

            for (int v = 0; v < n; ++v) {
                if (u != v && graph[u][v] != INF) {
                    if (!visited[v] && dist[v] > current_dist + graph[u][v]) {
                        dist[v] = current_dist + graph[u][v];
                        prev[v] = u;
                        pq.push({dist[v], v});//updating with new reduced distance in the priority queue
                    }
                }
            }
        }
        
        printLSRTable(src, dist, prev);
    }
}

vector<vector<int>> readGraphFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        exit(1);
    }
    
    int n;
    file >> n;
    vector<vector<int>> graph(n, vector<int>(n));

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            file >> graph[i][j];

    file.close();
    return graph;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    string filename = argv[1];
    vector<vector<int>> graph = readGraphFromFile(filename);

    cout << "\n--- Distance Vector Routing Simulation ---\n";
    simulateDVR(graph);

    cout << "\n--- Link State Routing Simulation ---\n";
    simulateLSR(graph);

    return 0;
}

