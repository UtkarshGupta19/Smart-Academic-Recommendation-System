# Smart Academic Recommendation System (SARS)

**Course:** Data Structures and Algorithms – II (BCSE0401)  
**Student:** Utkarsh Kumar Gupta (Roll No: 2501330100396)  
**Institution:** NIET Greater Noida (B.Tech CSE-F)  
**Faculty Evaluator:** Mr. Shamshad Ali  
**SDG Focus:** SDG 4 – Quality Education  

---

## 📌 Project Overview
SARS addresses manual advising bottlenecks by using core Data Structures & Algorithms concepts to plan, index, and optimize degree pathways:
- **Unit 1 (Catalog Indexing):** Self-balancing **AVL Tree** for $O(\log n)$ course search and sorted traversal.
- **Unit 2 (Curriculum Sequencing):** Directed Graph (Adjacency List) running **BFS** for fewest-semester degree paths and **Dijkstra's Algorithm** for minimum-effort academic routes.
- **Unit 3 (Elective Optimization):** Dynamic Programming **0/1 Knapsack** to select maximum-interest electives within credit boundaries.

---

## ⚙️ Algorithm Complexities

| Concept | Project Application | Time Complexity | Space Complexity |
|---|---|---|---|
| **AVL Tree** | Course catalog indexing & searching | $O(\log n)$ | $O(n)$ |
| **BFS** | Fewest-semester pathway lookup | $O(V + E)$ | $O(V)$ |
| **Dijkstra** | Workload/effort-optimized pathway | $O((V + E) \log V)$ | $O(V)$ |
| **0/1 Knapsack** | Credit-constrained elective optimization | $O(n \cdot W)$ | $O(n \cdot W)$ |

---

## 🚀 Execution Instructions

Run the prototype using Python 3:
```bash
python prototype_sars.py