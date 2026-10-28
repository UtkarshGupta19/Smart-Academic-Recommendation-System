const app = require('./server');
const http = require('http');

const PORT = 3899;
const server = http.createServer(app);

async function runTests() {
  server.listen(PORT, async () => {
    console.log(`Test server running on port ${PORT}`);
    try {
      // 1. Test /api/health
      const healthRes = await fetch(`http://localhost:${PORT}/api/health`);
      const health = await healthRes.json();
      console.log('Health check:', health.status, 'Engine:', health.engine);
      if (health.status !== 'online' || health.engine !== 'compiled') throw new Error('Health test failed');

      // 2. Test /api/courses (AVL inorder)
      const coursesRes = await fetch(`http://localhost:${PORT}/api/courses`);
      const courses = await coursesRes.json();
      console.log('AVL In-Order Courses:', courses.totalCourses, 'courses loaded, Tree Height:', courses.treeHeight);
      if (!courses.coursesSorted || courses.coursesSorted.length === 0) throw new Error('AVL courses test failed');

      // 2b. Test /api/courses/search/:id (AVL Search with Path)
      const searchRes = await fetch(`http://localhost:${PORT}/api/courses/search/CS301`);
      const searchData = await searchRes.json();
      console.log('AVL Search CS301 found:', searchData.found, 'Search Path:', searchData.searchPath, 'Comparisons:', searchData.comparisons);
      if (!searchData.found || !Array.isArray(searchData.searchPath) || searchData.comparisons < 1) {
        throw new Error('AVL Search path tracking test failed');
      }

      // 3. Test /api/recommend (Max-Heap & Score Consistency)
      const recRes = await fetch(`http://localhost:${PORT}/api/recommend`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          completedCourses: ['CS101', 'CS102', 'CS201'],
          interests: ['AI/ML'],
          gpa: 3.8,
          creditBudget: 12
        })
      });
      const rec = await recRes.json();
      console.log('Max-Heap Recommendations count:', rec.recommendations ? rec.recommendations.length : 0);
      if (!rec.recommendations || rec.recommendations.length === 0) throw new Error('Recommendation test failed');

      // Verify mathematical score consistency on all recommended courses
      rec.recommendations.forEach(item => {
        const componentSum = Math.round((item.trackScore + item.interestScore + item.gpaFitScore + item.prereqScore + item.budgetScore) * 10) / 10;
        const total = Math.round(item.totalScore * 10) / 10;
        if (Math.abs(componentSum - total) > 0.01) {
          throw new Error(`Score inconsistency on ${item.course.id}: sum=${componentSum} != total=${total}`);
        }
        if (!item.rationale || item.rationale.length === 0) {
          throw new Error(`Missing rationale on ${item.course.id}`);
        }
      });
      console.log('Max-Heap mathematical score consistency verified across all recommendations!');

      // 4. Test /api/learning-path (BFS vs Dijkstra)
      const pathRes = await fetch(`http://localhost:${PORT}/api/learning-path`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          start: 'CS101',
          target: 'CS403'
        })
      });
      const pathData = await pathRes.json();
      console.log('Learning Path BFS found:', pathData.bfsFewestHops.found, 'Path:', pathData.bfsFewestHops.path);
      console.log('Learning Path Dijkstra effort:', pathData.dijkstraMinEffort.totalEffort, 'Path:', pathData.dijkstraMinEffort.path);
      if (!pathData.bfsFewestHops.found || !pathData.dijkstraMinEffort.found) throw new Error('Learning path test failed');
      if (!pathData.bfsFewestHops.visitedOrder || !pathData.dijkstraMinEffort.visitedOrder) {
        throw new Error('Learning path visited orders missing');
      }

      // 5. Test /api/knapsack (0/1 Knapsack DP)
      const knapRes = await fetch(`http://localhost:${PORT}/api/knapsack`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ creditBudget: 10 })
      });
      const knap = await knapRes.json();
      console.log('Knapsack DP maxBenefit:', knap.maxBenefit, 'Credits used:', knap.creditsUsed);
      if (typeof knap.maxBenefit !== 'number' || typeof knap.creditsUsed !== 'number') {
        throw new Error('Knapsack test failed');
      }

      // 6. Test /api/mst (Kruskal & Prim MST)
      const mstRes = await fetch(`http://localhost:${PORT}/api/mst`);
      const mst = await mstRes.json();
      console.log('MST Kruskal weight:', mst.kruskal.totalWeight, 'Prim weight:', mst.prim.totalWeight, 'Weights match:', mst.weightsMatch);
      if (!mst.weightsMatch) throw new Error('MST test failed');

      // 7. Test /api/graph/algorithms (Floyd-Warshall C++ Integration)
      const graphRes = await fetch(`http://localhost:${PORT}/api/graph/algorithms?start=CS101`);
      const graphData = await graphRes.json();
      console.log('Floyd-Warshall all-pairs matrix dimensions:', graphData.floydWarshall ? graphData.floydWarshall.nodes.length : 0);
      if (!graphData.floydWarshall || !Array.isArray(graphData.floydWarshall.matrix)) {
        throw new Error('Floyd-Warshall C++ matrix test failed');
      }

      console.log('\n>>> ALL API & C++ ENGINE INTEGRATION TESTS PASSED! <<<\n');
      server.close();
      process.exit(0);
    } catch (err) {
      console.error('Integration test failure:', err);
      server.close();
      process.exit(1);
    }
  });
}

runTests();
