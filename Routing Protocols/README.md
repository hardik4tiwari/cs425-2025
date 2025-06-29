# Routing Simulation: DVR & LSR

This Assignment simulates **Distance Vector Routing (DVR)** and **Link State Routing (LSR)** algorithms using an adjacency matrix input to build and display routing tables.


##  How to Run

### 1. Compile the Code
Make sure you're in the same directory as the `Makefile` and source file. Run in the Linux environment:
```bash
make
```
This creates an executable named `routing_sim`.

### 2. Run the Executable
```bash
./routing_sim input.txt
```


##  Explanation of Completed TODOs

### Distance Vector Routing (`simulateDVR()`)

#### Initialization:
Each node initializes its routing table with direct connection costs. If no direct connection exists, cost is set to `INF` and next hop is `-1`.
```cpp
dist[i][j] = graph[i][j];
nextHop[i][j] = (i != j && graph[i][j] != INF) ? j : -1;
```

#### Update Loop:
Each node checks if it can find a cheaper path to a destination through an intermediate node. This is repeated until no further updates occur:
```cpp
for (int k = 0; k < n; ++k) {
    if (k == i || prev_dist[i][k] == INF) continue;
    int possible_cost = prev_dist[i][k] + prev_dist[k][j];
    if (possible_cost < min_cost) {
        min_cost = possible_cost;
        min_hop = k;
    }
}
```
This mimics the Bellman-Ford approach.

### Link State Routing (`simulateLSR()`)

#### Initialization:
Each source node starts with distances initialized to `INF` and uses a priority queue to implement Dijkstra's algorithm.
```cpp
dist[src] = 0;
pq.push({0, src});
```

#### Dijkstra's Algorithm:
The node with the smallest distance is selected from the priority queue. Distances and predecessors are updated:
```cpp
for (int v = 0; v < n; ++v) {
    if (!visited[v] && dist[v] > current_dist + graph[u][v]) {
        dist[v] = current_dist + graph[u][v];
        prev[v] = u;
        pq.push({dist[v], v});
    }
}
```

#### Next Hop Calculation:
After distances are calculated, the next hop is traced using the `prev[]` array:
```cpp
int hop = i;
while (prev[hop] != src && prev[hop] != -1)
    hop = prev[hop];
```

