# Professional Social Network Analyzer (C)

A console-based, graph-driven simulation of a LinkedIn-style professional network, implemented entirely in **C**. The project models users, their friendships, and career/mentorship relationships as a graph, and applies a wide range of classic data structures and graph algorithms to analyze the network.

## Overview

Each user is a node with a profile (name, school, college, company, hobbies) and two kinds of relationships:

- **Connections (undirected, weighted)** — represent friendships/professional links. Edge weight is computed automatically based on shared background (same school, college, company, or hobby reduces the weight, making "closer" connections cheaper).
- **Career edges (directed)** — represent mentorship/career progression links, used for topological sorting.

The program runs a single `main()` demo that builds a sample network of 7 users, performs various operations, and prints the results of each algorithm in a readable banner-style format.

## Data Structures Used

- **Adjacency List** (linked list `EdgeNode`) — stores weighted connections per user.
- **Custom Queue** (array-based, `push`/`pop`/`Front`) — used for BFS traversals.
- **Custom Binary Min-Heap / Priority Queue** (`PriorityQueue`) — used for Prim's MST.
- **Disjoint Set Union (DSU / Union-Find)** with union by rank — used for Kruskal's MST and cycle detection.
- **Static arrays acting as a "slot" allocator** — supports user deletion and slot reuse without dynamic resizing.

## Features / Algorithms Implemented

| Feature | Description |
|---|---|
| `addUser` / `deleteUser` | Add or remove a user; deleted slots are reused for new users. |
| `addConnection` / `removeConnection` | Add or remove a weighted, bidirectional friendship edge. |
| `addCareerEdge` | Add a directed career/mentorship edge between two users. |
| `bfsLevels` | BFS traversal grouping friends by "degree of separation" levels. |
| `dfsTraversal` | DFS traversal of the entire network (including disconnected components). |
| `topologicalSort` | Kahn's BFS-based topological sort over career edges, with cycle detection guard. |
| `primMST` | Prim's algorithm (heap-based) to compute the Minimum Spanning Tree / "minimum cost network". |
| `kruskalMST` | Kruskal's algorithm (sort edges + DSU) to compute the MST. |
| `dijkstra` | Dijkstra's algorithm for single-source shortest paths, with path reconstruction. |
| `floydWarshall` | All-pairs shortest paths with full distance matrix and path reconstruction. |
| `searchByBFS` / `searchByDFS` | Search users by attribute (name, school, college, company, hobby) using BFS or DFS traversal order. |
| `checkConnection` (`isConnected`) | Check whether two users are connected via BFS. |
| `printDegreeOfSeparation` | Compute the shortest "degree of separation" (hop count) between two users. |
| `detectCycle` | Detect cycles in the friendship graph using both DFS and Union-Find. |
| `allPaths` | Enumerate all simple paths between two users using backtracking DFS. |
| `graphDiameter` | Compute the graph diameter (maximum degree of separation) via repeated BFS. |
| `displayContactList` | Print the full adjacency list with edge weights. |
| `displayUserNetwork` | Show a user's direct connections and their full reachable network (BFS levels). |
| `findMates` | Find users sharing the same school, college, or company as a given user. |
| `hobbyGroups` | Group all users into "communities" based on shared hobbies. |
| `latestConnections` | Show a user's connections ordered by recency (most recently added first), plus the same for each of their friends. |
| `displayProfile` / `displayProfilesByName` | Print a formatted user profile, or search profiles by (case-insensitive) name substring. |

## Edge Weight Logic

Connection weights start at `BASE_WEIGHT` (5) and are reduced by 1 (down to a minimum of 1) for each shared attribute between two users:

- Same college
- Same company
- Same school
- Any shared hobby

This means users with more in common are "closer" in the weighted graph, which directly affects MST and shortest-path results.

## How to Build & Run

```bash
gcc -o Linkedin Linkedin.c
./Linkedin
```

(Replace `Linkedin.c` with the actual source filename.)

## Sample Demo Data

The `main()` function pre-populates the network with 7 sample users (Kishan, Aman, Ravi, Neha, Rahul, Priya, Rohan), wires up friendship connections, career edges, and then runs through every algorithm above — including a demonstration of deleting a user (Priya), reusing her slot for a new user (Sneha), and re-running connectivity checks afterward.

## Constants / Limits

| Constant | Value | Meaning |
|---|---|---|
| `MAX_USERS` | 100 | Maximum number of users (active + inactive slots) |
| `MAX_HOBBIES` | 10 | Maximum hobbies per user |
| `MAX_NAME_LEN` | 64 | Max length for names/hobby strings |
| `MAX_STR_LEN` | 128 | Max length for school/college/company strings |
| `INF` | 500000 | "Infinity" sentinel used in shortest-path algorithms |
| `BASE_WEIGHT` | 5 | Starting weight for any new connection |

## Notes

- This is a single-file educational/demo program intended to showcase graph algorithms and classic data structures implemented from scratch in C (no external libraries beyond the standard library).
- User deletion does not shrink `userCount`; freed slots are simply marked inactive and reused by future `addUser` calls.

## License
This project is for educational purposes and is free to use or modify.
