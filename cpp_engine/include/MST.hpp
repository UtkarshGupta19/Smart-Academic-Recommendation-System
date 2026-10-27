#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <queue>
#include <algorithm>
#include <numeric>
#include "json.hpp"

using json = nlohmann::json;

struct MSTEdge {
    std::string u;
    std::string v;
    double weight;
    std::string label;

    bool operator<(const MSTEdge& other) const {
        return weight < other.weight;
    }

    bool operator>(const MSTEdge& other) const {
        return weight > other.weight;
    }

    json toJson() const {
        return json{{"u", u}, {"v", v}, {"weight", weight}, {"label", label}};
    }
};

// Disjoint Set Union (DSU) with Path Compression and Union by Rank
class DisjointSetUnion {
private:
    std::unordered_map<std::string, std::string> parent;
    std::unordered_map<std::string, int> rank;

public:
    void makeSet(const std::string& item) {
        parent[item] = item;
        rank[item] = 0;
    }

    std::string find(const std::string& item) {
        if (parent.find(item) == parent.end()) {
            makeSet(item);
            return item;
        }
        if (parent[item] != item) {
            parent[item] = find(parent[item]); // Path compression
        }
        return parent[item];
    }

    bool unite(const std::string& a, const std::string& b) {
        std::string rootA = find(a);
        std::string rootB = find(b);

        if (rootA == rootB) return false; // Cycle detected

        // Union by rank
        if (rank[rootA] < rank[rootB]) {
            parent[rootA] = rootB;
        } else if (rank[rootA] > rank[rootB]) {
            parent[rootB] = rootA;
        } else {
            parent[rootB] = rootA;
            rank[rootA]++;
        }
        return true;
    }
};

class MSTSolver {
public:
    // Kruskal's Algorithm with DSU
    static json kruskalMST(
        const std::vector<std::string>& vertices,
        const std::vector<MSTEdge>& edges
    ) {
        DisjointSetUnion dsu;
        for (const auto& v : vertices) {
            dsu.makeSet(v);
        }

        std::vector<MSTEdge> sortedEdges = edges;
        std::sort(sortedEdges.begin(), sortedEdges.end());

        std::vector<MSTEdge> mstEdges;
        double totalWeight = 0.0;
        json steps = json::array();
        int rejectedCycles = 0;

        for (const auto& edge : sortedEdges) {
            bool united = dsu.unite(edge.u, edge.v);
            if (united) {
                mstEdges.push_back(edge);
                totalWeight += edge.weight;
                steps.push_back({
                    {"action", "ACCEPTED"},
                    {"edge", edge.toJson()},
                    {"reason", "Connects disjoint academic components without forming a cycle."}
                });
            } else {
                rejectedCycles++;
                steps.push_back({
                    {"action", "REJECTED"},
                    {"edge", edge.toJson()},
                    {"reason", "Forms a cycle in the academic skill tree."}
                });
            }

            if (mstEdges.size() == vertices.size() - 1 && !vertices.empty()) {
                break;
            }
        }

        json mstEdgesJson = json::array();
        for (const auto& e : mstEdges) {
            mstEdgesJson.push_back(e.toJson());
        }

        return json{
            {"algorithm", "Kruskal's Algorithm"},
            {"description", "Constructs the Minimum Spanning Tree using Disjoint Set Union (DSU) and greedy edge sorting."},
            {"timeComplexity", "O(E log E) or O(E log V)"},
            {"spaceComplexity", "O(V + E)"},
            {"totalWeight", totalWeight},
            {"edgeCount", mstEdges.size()},
            {"rejectedCycles", rejectedCycles},
            {"mstEdges", mstEdgesJson},
            {"selectionSteps", steps}
        };
    }

    // Prim's Algorithm using Min-Priority Queue
    static json primMST(
        const std::vector<std::string>& vertices,
        const std::vector<MSTEdge>& edges,
        const std::string& startVertex = ""
    ) {
        if (vertices.empty()) {
            return json{{"totalWeight", 0.0}, {"mstEdges", json::array()}};
        }

        std::unordered_map<std::string, std::vector<std::pair<std::string, double>>> adj;
        for (const auto& v : vertices) {
            adj[v] = {};
        }
        for (const auto& e : edges) {
            adj[e.u].push_back({e.v, e.weight});
            adj[e.v].push_back({e.u, e.weight});
        }

        std::string root = startVertex.empty() ? vertices[0] : startVertex;
        std::unordered_set<std::string> inMST;
        std::vector<MSTEdge> mstEdges;
        double totalWeight = 0.0;

        // Min-heap storing {weight, {u, v}}
        using EdgeEntry = std::pair<double, std::pair<std::string, std::string>>;
        std::priority_queue<EdgeEntry, std::vector<EdgeEntry>, std::greater<EdgeEntry>> pq;

        inMST.insert(root);
        for (const auto& [neighbor, wt] : adj[root]) {
            pq.push({wt, {root, neighbor}});
        }

        json visitOrder = json::array();
        visitOrder.push_back(root);

        while (!pq.empty() && inMST.size() < vertices.size()) {
            auto [wt, uv] = pq.top();
            pq.pop();

            std::string u = uv.first;
            std::string v = uv.second;

            if (inMST.find(v) != inMST.end()) {
                continue; // Already in MST
            }

            inMST.insert(v);
            visitOrder.push_back(v);
            mstEdges.push_back({u, v, wt, "mst-edge"});
            totalWeight += wt;

            for (const auto& [nxt, nxtWt] : adj[v]) {
                if (inMST.find(nxt) == inMST.end()) {
                    pq.push({nxtWt, {v, nxt}});
                }
            }
        }

        json mstEdgesJson = json::array();
        for (const auto& e : mstEdges) {
            mstEdgesJson.push_back(e.toJson());
        }

        return json{
            {"algorithm", "Prim's Algorithm"},
            {"description", "Grows the Minimum Spanning Tree from an initial root node by greedily picking the lowest-weight cut edge."},
            {"timeComplexity", "O((V + E) log V)"},
            {"spaceComplexity", "O(V + E)"},
            {"startNode", root},
            {"totalWeight", totalWeight},
            {"edgeCount", mstEdges.size()},
            {"mstEdges", mstEdgesJson},
            {"visitedSequence", visitOrder}
        };
    }
};
