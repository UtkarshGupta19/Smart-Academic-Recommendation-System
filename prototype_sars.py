import heapq

# ==============================================================================
# UNIT 1: AVL TREE (Course Catalog Indexing - O(log N) Search/Insert)
# ==============================================================================
class AVLNode:
    def __init__(self, code, name, credits, difficulty):
        self.code = code
        self.name = name
        self.credits = credits
        self.difficulty = difficulty
        self.height = 1
        self.left = None
        self.right = None

class CourseCatalogAVL:
    def get_height(self, node):
        return node.height if node else 0

    def get_balance(self, node):
        return self.get_height(node.left) - self.get_height(node.right) if node else 0

    def rotate_right(self, y):
        x = y.left
        T2 = x.right
        x.right = y
        y.left = T2
        y.height = 1 + max(self.get_height(y.left), self.get_height(y.right))
        x.height = 1 + max(self.get_height(x.left), self.get_height(x.right))
        return x

    def rotate_left(self, x):
        y = x.right
        T2 = y.left
        y.left = x
        x.right = T2
        x.height = 1 + max(self.get_height(x.left), self.get_height(x.right))
        y.height = 1 + max(self.get_height(y.left), self.get_height(y.right))
        return y

    def insert(self, node, code, name, credits, difficulty):
        if not node:
            return AVLNode(code, name, credits, difficulty)
        if code < node.code:
            node.left = self.insert(node.left, code, name, credits, difficulty)
        elif code > node.code:
            node.right = self.insert(node.right, code, name, credits, difficulty)
        else:
            return node

        node.height = 1 + max(self.get_height(node.left), self.get_height(node.right))
        balance = self.get_balance(node)

        # Left Left
        if balance > 1 and code < node.left.code:
            return self.rotate_right(node)
        # Right Right
        if balance < -1 and code > node.right.code:
            return self.rotate_left(node)
        # Left Right
        if balance > 1 and code > node.left.code:
            node.left = self.rotate_left(node.left)
            return self.rotate_right(node)
        # Right Left
        if balance < -1 and code < node.right.code:
            node.right = self.rotate_right(node.right)
            return self.rotate_left(node)

        return node

    def search(self, node, code):
        if not node or node.code == code:
            return node
        if code < node.code:
            return self.search(node.left, code)
        return self.search(node.right, code)

    def inorder(self, node, result):
        if node:
            self.inorder(node.left, result)
            result.append(node)
            self.inorder(node.right, result)


# ==============================================================================
# UNIT 2: CURRICULUM GRAPH (BFS & Dijkstra - Path Sequencing)
# ==============================================================================
class CurriculumGraph:
    def __init__(self):
        # Adjacency list: course -> list of (prerequisite_or_next, effort_weight)
        self.adj = {}

    def add_course(self, code):
        if code not in self.adj:
            self.adj[code] = []

    def add_dependency(self, prereq, target, effort):
        self.add_course(prereq)
        self.add_course(target)
        self.adj[prereq].append((target, effort))

    def fewest_semesters_bfs(self, start, target):
        """BFS: Finds shortest academic path by semester hops."""
        queue = [(start, [start])]
        visited = set([start])

        while queue:
            curr, path = queue.pop(0)
            if curr == target:
                return path
            for neighbor, _ in self.adj.get(curr, []):
                if neighbor not in visited:
                    visited.add(neighbor)
                    queue.append((neighbor, path + [neighbor]))
        return None

    def lowest_effort_dijkstra(self, start, target):
        """Dijkstra: Finds path minimizing workload/effort score."""
        pq = [(0, start, [start])]
        min_efforts = {start: 0}

        while pq:
            effort, curr, path = heapq.heappop(pq)
            if curr == target:
                return effort, path

            if effort > min_efforts.get(curr, float('inf')):
                continue

            for neighbor, weight in self.adj.get(curr, []):
                new_effort = effort + weight
                if new_effort < min_efforts.get(neighbor, float('inf')):
                    min_efforts[neighbor] = new_effort
                    heapq.heappush(pq, (new_effort, neighbor, path + [neighbor]))
        return float('inf'), []


# ==============================================================================
# UNIT 3: DYNAMIC PROGRAMMING (0/1 Knapsack - Elective Credit Optimization)
# ==============================================================================
def optimize_electives_knapsack(electives, credit_budget):
    """
    electives: list of tuples (code, credits, fit_score)
    credit_budget: integer max credits allowed
    Returns: (max_score, selected_courses)
    """
    n = len(electives)
    dp = [[0 for _ in range(credit_budget + 1)] for _ in range(n + 1)]

    for i in range(1, n + 1):
        code, cred, score = electives[i - 1]
        for w in range(credit_budget + 1):
            if cred <= w:
                dp[i][w] = max(dp[i - 1][w], dp[i - 1][w - cred] + score)
            else:
                dp[i][w] = dp[i - 1][w]

    # Backtracking to reconstruct chosen electives
    selected = []
    w = credit_budget
    for i in range(n, 0, -1):
        if dp[i][w] != dp[i - 1][w]:
            selected.append(electives[i - 1])
            w -= electives[i - 1][1]

    return dp[n][credit_budget], selected


# ==============================================================================
# SARS INTEGRATED SYSTEM (The Live Prototype)
# ==============================================================================
class SARSPrototype:
    def __init__(self):
        self.catalog = CourseCatalogAVL()
        self.catalog_root = None
        self.graph = CurriculumGraph()

    def bootstrap_data(self):
        # Course Database: (code, name, credits, difficulty_score)
        courses = [
            ("CS101", "Programming Fundamentals", 4, 3),
            ("CS201", "Data Structures & Algorithms", 4, 5),
            ("CS202", "Discrete Mathematics", 3, 4),
            ("CS301", "Database Management Systems", 3, 3),
            ("CS302", "Design & Analysis of Algorithms", 4, 6),
            ("CS401", "Artificial Intelligence", 3, 5),
            ("CS402", "Machine Learning & Data Mining", 4, 7),
            ("CS403", "Cloud & Distributed Systems", 3, 4),
            ("CS404", "Information & Cyber Security", 3, 4),
            ("CS405", "Big Data Analytics", 4, 6),
        ]
        for code, name, cred, diff in courses:
            self.catalog_root = self.catalog.insert(self.catalog_root, code, name, cred, diff)

        # Prerequisite network with effort costs
        dependencies = [
            ("CS101", "CS201", 3),
            ("CS101", "CS202", 2),
            ("CS201", "CS301", 3),
            ("CS201", "CS302", 4),
            ("CS202", "CS302", 2),
            ("CS302", "CS401", 4),
            ("CS302", "CS402", 5),
            ("CS301", "CS403", 3),
            ("CS301", "CS405", 4),
            ("CS401", "CS402", 2),
        ]
        for u, v, weight in dependencies:
            self.graph.add_dependency(u, v, weight)

    def display_catalog(self):
        result = []
        self.catalog.inorder(self.catalog_root, result)
        print("\n--- Current Course Catalog (AVL In-Order) ---")
        print(f"{'Code':<8} | {'Course Title':<32} | {'Credits':<8} | {'Difficulty'}")
        print("-" * 60)
        for c in result:
            print(f"{c.code:<8} | {c.name:<32} | {c.credits:<8} | {c.difficulty}/10")

    def run_path_planning(self, start, target):
        print(f"\nEvaluating paths from {start} to {target}...")
        # 1. BFS Path
        bfs_path = self.graph.fewest_semesters_bfs(start, target)
        if bfs_path:
            print(f"[BFS - Fewest Semesters]: {' -> '.join(bfs_path)} ({len(bfs_path)-1} semesters)")
        else:
            print("[BFS]: Target unreachable.")

        # 2. Dijkstra Path
        effort, dijkstra_path = self.graph.lowest_effort_dijkstra(start, target)
        if dijkstra_path:
            print(f"[Dijkstra - Lowest Effort]: {' -> '.join(dijkstra_path)} (Total Effort: {effort} pts)")
        else:
            print("[Dijkstra]: Target unreachable.")

    def run_elective_optimizer(self, credit_limit=8):
        print(f"\n--- Elective Selection (0/1 Knapsack Optimization) ---")
        # Elective pool: (Code, Credits, Student Fit/Score)
        electives_pool = [
            ("CS401", 3, 75),
            ("CS402", 4, 90),
            ("CS403", 3, 65),
            ("CS404", 3, 70),
            ("CS405", 4, 85),
        ]
        max_score, selected = optimize_electives_knapsack(electives_pool, credit_limit)
        print(f"Credit Budget: {credit_limit} credits")
        print(f"Optimal Fit Score Achieved: {max_score}")
        print("Selected Electives:")
        total_cred = 0
        for code, cred, score in selected:
            print(f"  • {code} ({cred} credits) - Fit Score: {score}")
            total_cred += cred
        print(f"Total Credits Utilized: {total_cred}/{credit_limit}")


def main():
    sars = SARSPrototype()
    sars.bootstrap_data()

    while True:
        print("\n=================================================")
        print(" SMART ACADEMIC RECOMMENDATION SYSTEM (SARS)")
        print("=================================================")
        print("1. View Indexed Course Catalog (AVL Tree)")
        print("2. Search Specific Course (AVL Tree O(log n))")
        print("3. Career Milestone Path Sequencing (BFS vs Dijkstra)")
        print("4. Elective Credit Optimizer (0/1 Knapsack DP)")
        print("5. Run Full Demonstration Pipeline")
        print("6. Exit")
        choice = input("Enter choice (1-6): ").strip()

        if choice == "1":
            sars.display_catalog()
        elif choice == "2":
            code = input("Enter course code (e.g., CS302): ").strip().upper()
            node = sars.catalog.search(sars.catalog_root, code)
            if node:
                print(f"\nFound: {node.code} - {node.name} (Credits: {node.credits}, Difficulty: {node.difficulty})")
            else:
                print(f"\nCourse code '{code}' not found in catalog.")
        elif choice == "3":
            start = input("Enter starting course (e.g., CS101): ").strip().upper()
            target = input("Enter target milestone (e.g., CS402): ").strip().upper()
            sars.run_path_planning(start, target)
        elif choice == "4":
            try:
                budget = int(input("Enter elective credit limit (e.g., 6, 8, or 10): "))
                sars.run_elective_optimizer(budget)
            except ValueError:
                print("Invalid integer entered.")
        elif choice == "5":
            print("\n>>> Running Demonstration Pipeline <<<")
            sars.display_catalog()
            sars.run_path_planning("CS101", "CS402")
            sars.run_elective_optimizer(8)
        elif choice == "6":
            print("Exiting SARS Prototype.")
            break
        else:
            print("Invalid selection. Try again.")

if __name__ == "__main__":
    main()