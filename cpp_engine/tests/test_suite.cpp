#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>
#include "Course.hpp"
#include "AVLTree.hpp"
#include "Graph.hpp"
#include "MaxHeap.hpp"
#include "DPAlgorithms.hpp"
#include "MST.hpp"

// Lightweight test assertion framework
int totalTests = 0;
int passedTests = 0;

#define TEST(name) \
    void test_##name(); \
    struct Runner_##name { \
        Runner_##name() { \
            totalTests++; \
            std::cout << "[RUNNING] " << #name << " ... "; \
            try { \
                test_##name(); \
                passedTests++; \
                std::cout << "PASSED\n"; \
            } catch (const std::exception& e) { \
                std::cout << "FAILED: " << e.what() << "\n"; \
            } catch (...) { \
                std::cout << "FAILED: Unknown error\n"; \
            } \
        } \
    } runner_##name; \
    void test_##name()

// 1. AVL Tree Tests
TEST(AVLTree_Rotations_And_Balance) {
    AVLTree tree;
    // Insert in order that causes RR, LL, RL, and LR rotations
    Course c1{"CS101", "Intro CS", "Core", 3, 1, 1, {}, {}, ""};
    Course c2{"CS102", "Data Structures", "Core", 4, 2, 2, {}, {}, ""};
    Course c3{"CS103", "Algorithms", "Core", 4, 3, 3, {}, {}, ""};
    Course c4{"CS100", "Math for CS", "Core", 3, 2, 1, {}, {}, ""};
    Course c5{"CS099", "Pre-CS", "Core", 2, 1, 1, {}, {}, ""};

    tree.insert(c1);
    tree.insert(c2);
    tree.insert(c3); // Causes Left Rotation (RR)
    tree.insert(c4);
    tree.insert(c5); // Causes Right Rotation (LL)

    assert(tree.size() == 5);
    assert(tree.getHeight() <= 3); // Height must be logarithmic

    // In-order traversal must be strictly sorted
    auto inOrder = tree.getInOrder();
    assert(inOrder.size() == 5);
    for (size_t i = 1; i < inOrder.size(); ++i) {
        assert(inOrder[i - 1].id < inOrder[i].id);
    }

    // Search tests
    assert(tree.search("CS102") != nullptr);
    assert(tree.search("CS999") == nullptr);

    // Search with path tracking
    std::vector<std::string> searchPath;
    assert(tree.search("CS102", searchPath) != nullptr);
    assert(!searchPath.empty());
    assert(searchPath.back() == "CS102");

    // Range search
    auto range = tree.getRange("CS100", "CS102");
    assert(range.size() == 3);
}

TEST(AVLTree_EdgeCases) {
    AVLTree emptyTree;
    assert(emptyTree.isEmpty());
    assert(emptyTree.size() == 0);
    assert(emptyTree.search("NON_EXISTENT") == nullptr);
    assert(emptyTree.getInOrder().empty());
}

// 2. Graph Algorithms Tests
TEST(Graph_BFS_And_Dijkstra) {
    AcademicGraph g;
    Course c1{"CS101", "Intro", "Core", 3, 1, 1, {}, {}, ""};
    Course c2{"CS102", "DS", "Core", 4, 2, 2, {}, {}, ""};
    Course c3{"CS201", "Algos", "Core", 4, 3, 3, {}, {}, ""};
    Course c4{"CS301", "AI", "AI/ML", 4, 4, 4, {}, {}, ""};

    g.addCourse(c1);
    g.addCourse(c2);
    g.addCourse(c3);
    g.addCourse(c4);

    // Path 1: CS101 -> CS301 directly (effort 10)
    g.addEdge("CS101", "CS301", 10.0);
    // Path 2: CS101 -> CS102 (2.0) -> CS201 (2.0) -> CS301 (2.0) = total effort 6.0
    g.addEdge("CS101", "CS102", 2.0);
    g.addEdge("CS102", "CS201", 2.0);
    g.addEdge("CS201", "CS301", 2.0);

    // BFS should choose Path 1 (1 hop)
    json bfs = g.bfsPath("CS101", "CS301");
    assert(bfs["found"] == true);
    assert(bfs["edgeCount"] == 1);
    assert(bfs["path"].size() == 2);
    assert(bfs["path"][0] == "CS101" && bfs["path"][1] == "CS301");

    // Dijkstra should choose Path 2 (lower total effort: 6.0 vs 10.0)
    json dijkstra = g.dijkstraPath("CS101", "CS301");
    assert(dijkstra["found"] == true);
    assert(std::abs(dijkstra["totalEffort"].get<double>() - 6.0) < 1e-4);
    assert(dijkstra["path"].size() == 4);
    assert(dijkstra["path"][1] == "CS102");
}

TEST(Graph_Unreachable_And_Cycle_Detection) {
    AcademicGraph g;
    Course c1{"A", "Course A", "Core", 3, 1, 1, {}, {}, ""};
    Course c2{"B", "Course B", "Core", 3, 1, 1, {}, {}, ""};
    Course c3{"C", "Course C", "Core", 3, 1, 1, {}, {}, ""};

    g.addCourse(c1);
    g.addCourse(c2);
    g.addCourse(c3);

    // Disconnected edge
    g.addEdge("A", "B", 1.0);
    // Node C is isolated

    json bfs = g.bfsPath("A", "C");
    assert(bfs["found"] == false);

    json dijkstra = g.dijkstraPath("A", "C");
    assert(dijkstra["found"] == false);

    // Connected components: should have 2 components ({A, B} and {C})
    json cc = g.getConnectedComponents();
    assert(cc["componentCount"] == 2);

    // Add cycle: B -> A
    g.addEdge("B", "A", 1.0);
    json dfs = g.dfsTraverse();
    assert(dfs["hasCycle"] == true);
}

TEST(Graph_BellmanFord_And_FloydWarshall) {
    AcademicGraph g;
    Course c1{"A", "A", "Core", 3, 1, 1, {}, {}, ""};
    Course c2{"B", "B", "Core", 3, 1, 1, {}, {}, ""};
    Course c3{"C", "C", "Core", 3, 1, 1, {}, {}, ""};

    g.addCourse(c1);
    g.addCourse(c2);
    g.addCourse(c3);

    g.addEdge("A", "B", 4.0);
    g.addEdge("B", "C", -2.0); // Arbitrary/negative edge
    g.addEdge("A", "C", 5.0);

    json bf = g.bellmanFord("A");
    assert(bf["hasNegativeCycle"] == false);
    assert(std::abs(bf["distances"]["C"].get<double>() - 2.0) < 1e-4);

    json fw = g.floydWarshall();
    assert(fw["nodes"].size() == 3);
}

// 3. MaxHeap and Recommender Tests
TEST(MaxHeap_Operations) {
    MaxHeap heap;
    assert(heap.empty());
    assert(heap.size() == 0);

    Course dummy{"C", "Name", "Core", 3, 2, 1, {}, {}, ""};
    RecommendationItem item1{dummy, 50.0, 10, 10, 10, 10, 10, true, {}, "R1", {}};
    RecommendationItem item2{dummy, 90.0, 30, 15, 20, 15, 10, true, {}, "R2", {}};
    RecommendationItem item3{dummy, 70.0, 20, 10, 15, 15, 10, true, {}, "R3", {}};

    heap.push(item1);
    heap.push(item2);
    heap.push(item3);

    assert(heap.size() == 3);
    assert(heap.top().totalScore == 90.0);

    auto top1 = heap.pop();
    assert(top1.totalScore == 90.0);

    auto top2 = heap.pop();
    assert(top2.totalScore == 70.0);

    auto top3 = heap.pop();
    assert(top3.totalScore == 50.0);

    assert(heap.empty());
}

TEST(CourseRecommender_PrerequisiteFilter_And_ScoreConsistency) {
    Course intro{"CS101", "Intro", "Core", 3, 1, 1, {}, {"python"}, ""};
    Course advAI{"CS401", "Advanced AI", "AI/ML", 4, 5, 4, {"CS101", "CS201"}, {"ai", "deep-learning"}, ""};
    Course dataStruct{"CS201", "Data Structures", "Core", 4, 3, 2, {"CS101"}, {"arrays", "trees"}, ""};

    std::vector<Course> catalog = {intro, advAI, dataStruct};
    std::unordered_set<std::string> completed = {"CS101"}; // CS201 missing
    std::vector<std::string> interests = {"AI/ML"};

    json rec = CourseRecommender::rankCourses(catalog, completed, interests, 3.8, 15);
    // advAI should be locked / ineligible because CS201 is missing
    assert(rec["lockedCourses"].size() == 1);
    assert(rec["lockedCourses"][0]["course"]["id"] == "CS401");
    assert(rec["lockedCourses"][0]["isEligible"] == false);

    // Verify mathematical score consistency on all generated recommendations
    for (const auto& item : rec["recommendations"]) {
        double expectedSum = item["trackScore"].get<double>() +
                             item["interestScore"].get<double>() +
                             item["gpaFitScore"].get<double>() +
                             item["prereqScore"].get<double>() +
                             item["budgetScore"].get<double>();
        double total = item["totalScore"].get<double>();
        assert(std::abs(expectedSum - total) < 1e-4);
        assert(!item["rationale"].get<std::string>().empty());
    }
}

// 4. DP Algorithms Tests
TEST(Knapsack_ElectiveOptimization) {
    Course c1{"E1", "Cloud Computing", "Systems", 3, 3, 3, {}, {}, ""};
    Course c2{"E2", "Computer Vision", "AI/ML", 4, 4, 4, {}, {}, ""};
    Course c3{"E3", "Cyber Law", "Core", 2, 2, 2, {}, {}, ""};

    std::vector<Course> electives = {c1, c2, c3};
    std::vector<double> benefits = {30.0, 50.0, 20.0}; // E1(3 cr -> 30), E2(4 cr -> 50), E3(2 cr -> 20)

    // Budget of 6 credits: optimal is E2 (4 cr, 50 val) + E3 (2 cr, 20 val) = 70 val, 6 credits
    json res = DPAlgorithms::knapsackElectiveOptimization(electives, benefits, 6);
    assert(std::abs(res["maxBenefit"].get<double>() - 70.0) < 1e-4);
    assert(res["creditsUsed"] == 6);
    assert(res["selectedCourses"].size() == 2);

    // Edge case: 0 budget
    json zeroBudget = DPAlgorithms::knapsackElectiveOptimization(electives, benefits, 0);
    assert(zeroBudget["maxBenefit"] == 0);
}

TEST(LCS_StringAlignment) {
    std::string s1 = "MACHINE LEARNING";
    std::string s2 = "DEEP LEARNING";

    json lcs = DPAlgorithms::longestCommonSubsequence(s1, s2);
    // Common subsequence should end with " LEARNING" (length >= 9)
    assert(lcs["lcsLength"].get<int>() >= 9);

    // Edge case: Empty strings
    json emptyLcs = DPAlgorithms::longestCommonSubsequence("", "TEST");
    assert(emptyLcs["lcsLength"] == 0);
}

TEST(ResourceAllocation_DP) {
    std::vector<std::string> names = {"Compiler Design", "Web Development"};
    std::vector<int> diffs = {5, 2}; // Higher difficulty course gets more benefit per hour initially

    json alloc = DPAlgorithms::resourceAllocationDP(names, diffs, 10);
    assert(alloc["allocations"].size() == 2);
    assert(alloc["maxKnowledgeGain"].get<double>() > 0.0);
    int totalAllocated = 0;
    for (const auto& a : alloc["allocations"]) {
        totalAllocated += a["hoursAllocated"].get<int>();
    }
    assert(totalAllocated == 10);
}

// 5. MST Algorithms Tests
TEST(MST_Kruskal_And_Prim_Equivalence) {
    std::vector<std::string> vertices = {"A", "B", "C", "D"};
    std::vector<MSTEdge> edges = {
        {"A", "B", 1.0, ""},
        {"B", "C", 2.0, ""},
        {"A", "C", 4.0, ""},
        {"C", "D", 1.0, ""},
        {"B", "D", 3.0, ""}
    };

    json kruskal = MSTSolver::kruskalMST(vertices, edges);
    json prim = MSTSolver::primMST(vertices, edges, "A");

    // MST should take edges with weight 1 (A-B), 1 (C-D), and 2 (B-C), total = 4.0
    double wKruskal = kruskal["totalWeight"].get<double>();
    double wPrim = prim["totalWeight"].get<double>();

    assert(std::abs(wKruskal - 4.0) < 1e-4);
    assert(std::abs(wPrim - 4.0) < 1e-4);
    assert(std::abs(wKruskal - wPrim) < 1e-4);
    assert(kruskal["edgeCount"] == 3);
}

int main() {
    std::cout << "\n============================================\n";
    std::cout << "  SARS C++17 Core DSA Automated Test Suite  \n";
    std::cout << "============================================\n\n";

    std::cout << "Summary: " << passedTests << " / " << totalTests << " test suites executed.\n";
    if (passedTests == totalTests) {
        std::cout << "ALL " << totalTests << " TESTS PASSED SUCCESSFULLY! \xE2\x9C\x93\n\n";
        return 0;
    } else {
        std::cerr << "FAILURES DETECTED: " << (totalTests - passedTests) << " failed.\n\n";
        return 1;
    }
}
