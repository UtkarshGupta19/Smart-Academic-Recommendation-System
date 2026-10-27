#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <algorithm>
#include <limits>
#include "Course.hpp"
#include "json.hpp"

using json = nlohmann::json;

struct GraphEdge {
    std::string to;
    double weight; // Effort / difficulty weight
    std::string label;

    json toJson() const {
        return json{{"to", to}, {"weight", weight}, {"label", label}};
    }
};

class AcademicGraph {
private:
    std::unordered_map<std::string, Course> courses;
    std::unordered_map<std::string, std::vector<GraphEdge>> adj;
    std::unordered_map<std::string, std::vector<GraphEdge>> revAdj; // For incoming prereqs

public:
    AcademicGraph() = default;

    void addCourse(const Course& c) {
        courses[c.id] = c;
        if (adj.find(c.id) == adj.end()) {
            adj[c.id] = {};
        }
        if (revAdj.find(c.id) == revAdj.end()) {
            revAdj[c.id] = {};
        }
    }

    void addEdge(const std::string& from, const std::string& to, double weight, const std::string& label = "prerequisite") {
        if (courses.find(from) == courses.end() || courses.find(to) == courses.end()) {
            return;
        }
        adj[from].push_back({to, weight, label});
        revAdj[to].push_back({from, weight, label});
    }

    const std::unordered_map<std::string, Course>& getCourses() const {
        return courses;
    }

    bool hasCourse(const std::string& id) const {
        return courses.find(id) != courses.end();
    }

    // BFS: Fewest-edge path (unweighted shortest path)
    json bfsPath(const std::string& start, const std::string& target) const {
        if (!hasCourse(start) || !hasCourse(target)) {
            return json{{"found", false}, {"error", "Start or target course not found"}};
        }

        std::unordered_map<std::string, std::string> parent;
        std::unordered_set<std::string> visited;
        std::vector<std::string> visitedOrder;
        std::queue<std::string> q;

        visited.insert(start);
        visitedOrder.push_back(start);
        q.push(start);
        parent[start] = "";

        bool found = false;
        while (!q.empty()) {
            std::string curr = q.front();
            q.pop();

            if (curr == target) {
                found = true;
                break;
            }

            auto it = adj.find(curr);
            if (it != adj.end()) {
                for (const auto& edge : it->second) {
                    if (visited.find(edge.to) == visited.end()) {
                        visited.insert(edge.to);
                        visitedOrder.push_back(edge.to);
                        parent[edge.to] = curr;
                        q.push(edge.to);
                    }
                }
            }
        }

        std::vector<std::string> path;
        if (found) {
            std::string curr = target;
            while (!curr.empty()) {
                path.push_back(curr);
                curr = parent[curr];
            }
            std::reverse(path.begin(), path.end());
        }

        return json{
            {"found", found},
            {"algorithm", "Breadth-First Search (BFS)"},
            {"description", "Finds the path with the minimum number of course hops (unweighted shortest path)."},
            {"path", path},
            {"edgeCount", found ? (static_cast<int>(path.size()) - 1) : -1},
            {"nodesVisited", visitedOrder.size()},
            {"visitedOrder", visitedOrder}
        };
    }

    // DFS: Depth-First Search with Cycle Detection & Topological Sort
    json dfsTraverse(const std::string& startNode = "") const {
        std::unordered_set<std::string> visited;
        std::unordered_set<std::string> recStack;
        std::vector<std::string> visitedOrder;
        std::vector<std::string> topoOrder;
        bool hasCycle = false;
        std::vector<std::string> cyclePath;

        auto dfsHelper = [&](auto& self, const std::string& u) -> void {
            visited.insert(u);
            recStack.insert(u);
            visitedOrder.push_back(u);

            auto it = adj.find(u);
            if (it != adj.end()) {
                for (const auto& edge : it->second) {
                    if (visited.find(edge.to) == visited.end()) {
                        self(self, edge.to);
                    } else if (recStack.find(edge.to) != recStack.end()) {
                        hasCycle = true;
                        cyclePath.push_back(u + " -> " + edge.to);
                    }
                }
            }

            recStack.erase(u);
            topoOrder.push_back(u);
        };

        if (!startNode.empty() && hasCourse(startNode)) {
            dfsHelper(dfsHelper, startNode);
        } else {
            // Traverse all nodes
            for (const auto& pair : courses) {
                if (visited.find(pair.first) == visited.end()) {
                    dfsHelper(dfsHelper, pair.first);
                }
            }
        }

        // Invert topoOrder for correct topological ordering
        std::reverse(topoOrder.begin(), topoOrder.end());

        return json{
            {"algorithm", "Depth-First Search (DFS)"},
            {"description", "Explores branches deeply. Detects prerequisite cycles and generates academic topological order."},
            {"visitedOrder", visitedOrder},
            {"hasCycle", hasCycle},
            {"cycleEdges", cyclePath},
            {"topologicalSort", hasCycle ? json::array() : json(topoOrder)}
        };
    }

    // Dijkstra's Algorithm: Shortest path based on difficulty/effort weights
    json dijkstraPath(const std::string& start, const std::string& target) const {
        if (!hasCourse(start) || !hasCourse(target)) {
            return json{{"found", false}, {"error", "Start or target course not found"}};
        }

        const double INF = std::numeric_limits<double>::infinity();
        std::unordered_map<std::string, double> dist;
        std::unordered_map<std::string, std::string> parent;
        std::vector<std::string> visitedOrder;

        for (const auto& pair : courses) {
            dist[pair.first] = INF;
        }

        using NodeDist = std::pair<double, std::string>;
        std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<NodeDist>> pq;

        dist[start] = 0.0;
        parent[start] = "";
        pq.push({0.0, start});

        std::unordered_set<std::string> visited;
        bool found = false;

        while (!pq.empty()) {
            auto [d, u] = pq.top();
            pq.pop();

            if (visited.find(u) != visited.end()) continue;
            visited.insert(u);
            visitedOrder.push_back(u);

            if (u == target) {
                found = true;
                break;
            }

            if (d > dist[u]) continue;

            auto it = adj.find(u);
            if (it != adj.end()) {
                for (const auto& edge : it->second) {
                    double newDist = dist[u] + edge.weight;
                    if (newDist < dist[edge.to]) {
                        dist[edge.to] = newDist;
                        parent[edge.to] = u;
                        pq.push({newDist, edge.to});
                    }
                }
            }
        }

        std::vector<std::string> path;
        if (found) {
            std::string curr = target;
            while (!curr.empty()) {
                path.push_back(curr);
                curr = parent[curr];
            }
            std::reverse(path.begin(), path.end());
        }

        return json{
            {"found", found},
            {"algorithm", "Dijkstra's Algorithm"},
            {"description", "Finds the path minimizing cumulative difficulty/effort edge weights using a Min-Priority Queue."},
            {"path", path},
            {"totalEffort", found ? dist[target] : -1.0},
            {"nodesVisited", visitedOrder.size()},
            {"visitedOrder", visitedOrder}
        };
    }

    // Bellman-Ford Algorithm: Handles arbitrary edge weights & detects negative cycles
    json bellmanFord(const std::string& start) const {
        if (!hasCourse(start)) {
            return json{{"error", "Start course not found"}};
        }

        const double INF = 1e9;
        std::unordered_map<std::string, double> dist;
        std::unordered_map<std::string, std::string> parent;

        for (const auto& pair : courses) {
            dist[pair.first] = INF;
            parent[pair.first] = "";
        }
        dist[start] = 0.0;

        size_t V = courses.size();
        // Relax edges V - 1 times
        for (size_t i = 1; i < V; ++i) {
            for (const auto& [u, edges] : adj) {
                if (dist[u] >= INF) continue;
                for (const auto& edge : edges) {
                    if (dist[u] + edge.weight < dist[edge.to]) {
                        dist[edge.to] = dist[u] + edge.weight;
                        parent[edge.to] = u;
                    }
                }
            }
        }

        // Check for negative weight cycles
        bool hasNegativeCycle = false;
        std::vector<std::string> negativeCycleNodes;
        for (const auto& [u, edges] : adj) {
            if (dist[u] >= INF) continue;
            for (const auto& edge : edges) {
                if (dist[u] + edge.weight < dist[edge.to]) {
                    hasNegativeCycle = true;
                    negativeCycleNodes.push_back(edge.to);
                }
            }
        }

        json distJson = json::object();
        for (const auto& [id, d] : dist) {
            if (d < INF) distJson[id] = d;
            else distJson[id] = "Unreachable";
        }

        return json{
            {"algorithm", "Bellman-Ford Algorithm"},
            {"source", start},
            {"description", "Calculates single-source shortest paths through dynamic edge relaxations and detects negative cycles."},
            {"distances", distJson},
            {"hasNegativeCycle", hasNegativeCycle},
            {"negativeCycleNodes", negativeCycleNodes}
        };
    }

    // Floyd-Warshall: All-pairs shortest path matrix
    json floydWarshall() const {
        std::vector<std::string> nodes;
        for (const auto& pair : courses) {
            nodes.push_back(pair.first);
        }
        std::sort(nodes.begin(), nodes.end());

        size_t n = nodes.size();
        std::unordered_map<std::string, size_t> nodeIdx;
        for (size_t i = 0; i < n; ++i) {
            nodeIdx[nodes[i]] = i;
        }

        const double INF = 1e9;
        std::vector<std::vector<double>> dist(n, std::vector<double>(n, INF));

        for (size_t i = 0; i < n; ++i) {
            dist[i][i] = 0.0;
        }

        for (const auto& [u, edges] : adj) {
            size_t uIdx = nodeIdx[u];
            for (const auto& edge : edges) {
                if (nodeIdx.find(edge.to) != nodeIdx.end()) {
                    size_t vIdx = nodeIdx[edge.to];
                    dist[uIdx][vIdx] = std::min(dist[uIdx][vIdx], edge.weight);
                }
            }
        }

        for (size_t k = 0; k < n; ++k) {
            for (size_t i = 0; i < n; ++i) {
                for (size_t j = 0; j < n; ++j) {
                    if (dist[i][k] < INF && dist[k][j] < INF) {
                        if (dist[i][k] + dist[k][j] < dist[i][j]) {
                            dist[i][j] = dist[i][k] + dist[k][j];
                        }
                    }
                }
            }
        }

        json matrix = json::array();
        for (size_t i = 0; i < n; ++i) {
            json row = json::array();
            for (size_t j = 0; j < n; ++j) {
                if (dist[i][j] >= INF) row.push_back("INF");
                else row.push_back(dist[i][j]);
            }
            matrix.push_back(row);
        }

        return json{
            {"algorithm", "Floyd-Warshall All-Pairs Shortest Path"},
            {"description", "Computes all-pairs shortest paths using dynamic programming in O(V^3)."},
            {"nodes", nodes},
            {"matrix", matrix}
        };
    }

    // Connected Components (Undirected / Weakly Connected)
    json getConnectedComponents() const {
        // Build undirected graph representation
        std::unordered_map<std::string, std::vector<std::string>> undirAdj;
        for (const auto& pair : courses) {
            undirAdj[pair.first] = {};
        }
        for (const auto& [u, edges] : adj) {
            for (const auto& edge : edges) {
                undirAdj[u].push_back(edge.to);
                undirAdj[edge.to].push_back(u);
            }
        }

        std::unordered_set<std::string> visited;
        std::vector<std::vector<std::string>> components;

        for (const auto& pair : courses) {
            const std::string& u = pair.first;
            if (visited.find(u) == visited.end()) {
                std::vector<std::string> comp;
                std::queue<std::string> q;
                visited.insert(u);
                q.push(u);

                while (!q.empty()) {
                    std::string curr = q.front();
                    q.pop();
                    comp.push_back(curr);

                    for (const auto& neighbor : undirAdj[curr]) {
                        if (visited.find(neighbor) == visited.end()) {
                            visited.insert(neighbor);
                            q.push(neighbor);
                        }
                    }
                }
                std::sort(comp.begin(), comp.end());
                components.push_back(comp);
            }
        }

        return json{
            {"algorithm", "Connected Components"},
            {"description", "Identifies isolated course clusters and independent academic disciplines."},
            {"componentCount", components.size()},
            {"components", components}
        };
    }

    // Graph visual data for UI
    json getVisualData() const {
        json nodesJson = json::array();
        for (const auto& [id, c] : courses) {
            nodesJson.push_back({
                {"id", id},
                {"name", c.name},
                {"track", c.track},
                {"credits", c.credits},
                {"difficulty", c.difficulty},
                {"semester", c.semester}
            });
        }

        json edgesJson = json::array();
        for (const auto& [u, edges] : adj) {
            for (const auto& edge : edges) {
                edgesJson.push_back({
                    {"from", u},
                    {"to", edge.to},
                    {"weight", edge.weight},
                    {"label", edge.label}
                });
            }
        }

        return json{
            {"nodes", nodesJson},
            {"edges", edgesJson}
        };
    }
};
