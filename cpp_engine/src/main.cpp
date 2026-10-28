#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_set>
#include "json.hpp"
#include "Course.hpp"
#include "AVLTree.hpp"
#include "Graph.hpp"
#include "MaxHeap.hpp"
#include "DPAlgorithms.hpp"
#include "MST.hpp"

using json = nlohmann::json;

// Helper to construct AVL Tree from course list
AVLTree buildAVLTree(const json& coursesJson) {
    AVLTree tree;
    for (const auto& item : coursesJson) {
        Course c = Course::fromJson(item);
        tree.insert(c);
    }
    return tree;
}

// Helper to construct Academic Graph from courses and edges
AcademicGraph buildGraph(const json& coursesJson, const json& edgesJson) {
    AcademicGraph g;
    for (const auto& item : coursesJson) {
        g.addCourse(Course::fromJson(item));
    }
    if (edgesJson.is_array()) {
        for (const auto& e : edgesJson) {
            std::string from = e.value("from", "");
            std::string to = e.value("to", "");
            double weight = e.value("weight", 1.0);
            std::string label = e.value("label", "prerequisite");
            g.addEdge(from, to, weight, label);
        }
    }
    return g;
}

int main(int argc, char* argv[]) {
    // Fast I/O
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    std::string inputStr;
    if (argc > 1 && std::string(argv[1]) == "--json" && argc > 2) {
        inputStr = argv[2];
    } else {
        std::stringstream ss;
        ss << std::cin.rdbuf();
        inputStr = ss.str();
    }

    if (inputStr.empty()) {
        json info = {
            {"engine", "SARS C++17 Core DSA Engine"},
            {"version", "1.0.0"},
            {"supportedCommands", {
                "catalog_build", "catalog_search", "catalog_inorder", "catalog_range", "avl_visual",
                "graph_bfs", "graph_dfs", "graph_dijkstra", "graph_bellman_ford", "graph_floyd_warshall", "graph_connected",
                "learning_path", "recommend",
                "knapsack", "lcs", "resource_allocation",
                "mst_kruskal", "mst_prim", "mst_compare"
            }}
        };
        std::cout << info.dump(2) << std::endl;
        return 0;
    }

    try {
        json req = json::parse(inputStr);
        std::string cmd = req.value("command", "");
        json data = req.value("data", json::object());
        json result;

        if (cmd == "catalog_build" || cmd == "catalog_inorder") {
            AVLTree tree = buildAVLTree(data.value("courses", json::array()));
            std::vector<Course> inOrder = tree.getInOrder();
            json list = json::array();
            for (const auto& c : inOrder) {
                list.push_back(c.toJson());
            }
            result = {
                {"totalCourses", tree.size()},
                {"treeHeight", tree.getHeight()},
                {"coursesSorted", list}
            };
        } else if (cmd == "catalog_search") {
            AVLTree tree = buildAVLTree(data.value("courses", json::array()));
            std::string id = data.value("id", "");
            std::vector<std::string> path;
            const Course* found = tree.search(id, path);
            if (found) {
                result = {
                    {"found", true},
                    {"course", found->toJson()},
                    {"searchPath", path},
                    {"comparisons", path.size()}
                };
            } else {
                result = {
                    {"found", false},
                    {"message", "Course ID not found in AVL tree"},
                    {"searchPath", path},
                    {"comparisons", path.size()}
                };
            }
        } else if (cmd == "catalog_range") {
            AVLTree tree = buildAVLTree(data.value("courses", json::array()));
            std::string low = data.value("low", "");
            std::string high = data.value("high", "");
            std::vector<Course> rangeCourses = tree.getRange(low, high);
            json list = json::array();
            for (const auto& c : rangeCourses) {
                list.push_back(c.toJson());
            }
            result = {
                {"low", low},
                {"high", high},
                {"count", rangeCourses.size()},
                {"courses", list}
            };
        } else if (cmd == "avl_visual") {
            AVLTree tree = buildAVLTree(data.value("courses", json::array()));
            result = tree.toVisualJson();
        } else if (cmd == "graph_bfs") {
            AcademicGraph g = buildGraph(data.value("courses", json::array()), data.value("edges", json::array()));
            result = g.bfsPath(data.value("start", ""), data.value("target", ""));
        } else if (cmd == "graph_dfs") {
            AcademicGraph g = buildGraph(data.value("courses", json::array()), data.value("edges", json::array()));
            result = g.dfsTraverse(data.value("start", ""));
        } else if (cmd == "graph_dijkstra") {
            AcademicGraph g = buildGraph(data.value("courses", json::array()), data.value("edges", json::array()));
            result = g.dijkstraPath(data.value("start", ""), data.value("target", ""));
        } else if (cmd == "graph_bellman_ford") {
            AcademicGraph g = buildGraph(data.value("courses", json::array()), data.value("edges", json::array()));
            result = g.bellmanFord(data.value("start", ""));
        } else if (cmd == "graph_floyd_warshall") {
            AcademicGraph g = buildGraph(data.value("courses", json::array()), data.value("edges", json::array()));
            result = g.floydWarshall();
        } else if (cmd == "graph_connected") {
            AcademicGraph g = buildGraph(data.value("courses", json::array()), data.value("edges", json::array()));
            result = g.getConnectedComponents();
        } else if (cmd == "learning_path") {
            // Compare BFS (fewest hops) vs Dijkstra (minimum total difficulty/effort)
            AcademicGraph g = buildGraph(data.value("courses", json::array()), data.value("edges", json::array()));
            std::string start = data.value("start", "");
            std::string target = data.value("target", "");

            json bfsRes = g.bfsPath(start, target);
            json dijkstraRes = g.dijkstraPath(start, target);

            result = {
                {"start", start},
                {"target", target},
                {"bfsFewestHops", bfsRes},
                {"dijkstraMinEffort", dijkstraRes},
                {"graphVisual", g.getVisualData()}
            };
        } else if (cmd == "recommend") {
            std::vector<Course> catalog;
            for (const auto& item : data.value("courses", json::array())) {
                catalog.push_back(Course::fromJson(item));
            }

            std::unordered_set<std::string> completed;
            for (const auto& cId : data.value("completedCourses", json::array())) {
                completed.insert(cId.get<std::string>());
            }

            std::vector<std::string> interests;
            for (const auto& intr : data.value("interests", json::array())) {
                interests.push_back(intr.get<std::string>());
            }

            double gpa = data.value("gpa", 3.5);
            int creditBudget = data.value("creditBudget", 18);
            int topK = data.value("topK", 8);

            result = CourseRecommender::rankCourses(catalog, completed, interests, gpa, creditBudget, topK);
        } else if (cmd == "knapsack") {
            std::vector<Course> electives;
            std::vector<double> benefitValues;

            for (const auto& item : data.value("electives", json::array())) {
                Course c = Course::fromJson(item);
                electives.push_back(c);
                double val = item.value("benefit", static_cast<double>(c.difficulty * 10 + c.credits * 5));
                benefitValues.push_back(val);
            }

            int budget = data.value("creditBudget", 12);
            result = DPAlgorithms::knapsackElectiveOptimization(electives, benefitValues, budget);
        } else if (cmd == "lcs") {
            std::string str1 = data.value("str1", "");
            std::string str2 = data.value("str2", "");
            result = DPAlgorithms::longestCommonSubsequence(str1, str2);
        } else if (cmd == "resource_allocation") {
            std::vector<std::string> names;
            std::vector<int> diffs;

            for (const auto& item : data.value("courses", json::array())) {
                names.push_back(item.value("name", "Course"));
                diffs.push_back(item.value("difficulty", 3));
            }

            int totalHours = data.value("totalHours", 20);
            result = DPAlgorithms::resourceAllocationDP(names, diffs, totalHours);
        } else if (cmd == "mst_kruskal" || cmd == "mst_prim" || cmd == "mst_compare") {
            std::vector<std::string> vertices;
            for (const auto& v : data.value("vertices", json::array())) {
                vertices.push_back(v.get<std::string>());
            }

            std::vector<MSTEdge> edges;
            for (const auto& e : data.value("edges", json::array())) {
                edges.push_back({
                    e.value("u", ""),
                    e.value("v", ""),
                    e.value("weight", 1.0),
                    e.value("label", "")
                });
            }

            if (cmd == "mst_kruskal") {
                result = MSTSolver::kruskalMST(vertices, edges);
            } else if (cmd == "mst_prim") {
                std::string start = data.value("startVertex", "");
                result = MSTSolver::primMST(vertices, edges, start);
            } else {
                json kruskal = MSTSolver::kruskalMST(vertices, edges);
                json prim = MSTSolver::primMST(vertices, edges, vertices.empty() ? "" : vertices[0]);
                result = {
                    {"kruskal", kruskal},
                    {"prim", prim},
                    {"weightsMatch", std::abs(kruskal["totalWeight"].get<double>() - prim["totalWeight"].get<double>()) < 1e-4}
                };
            }
        } else {
            std::cout << json{{"status", "error"}, {"error", "Unknown command: " + cmd}}.dump() << std::endl;
            return 1;
        }

        json output = {
            {"status", "success"},
            {"command", cmd},
            {"result", result}
        };

        std::cout << output.dump() << std::endl;
        return 0;

    } catch (const std::exception& e) {
        json err = {
            {"status", "error"},
            {"error", std::string("Exception occurred: ") + e.what()}
        };
        std::cout << err.dump() << std::endl;
        return 1;
    }
}
