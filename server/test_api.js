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

      // 3. Test /api/recommend (Max-Heap)
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

      // 5. Test /api/knapsack (0/1 Knapsack DP)
      const knapRes = await fetch(`http://localhost:${PORT}/api/knapsack`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ creditBudget: 10 })
      });
      const knap = await knapRes.json();
      console.log('Knapsack DP maxBenefit:', knap.maxBenefit, 'Credits used:', knap.creditsUsed);

      // 6. Test /api/mst (Kruskal & Prim MST)
      const mstRes = await fetch(`http://localhost:${PORT}/api/mst`);
      const mst = await mstRes.json();
      console.log('MST Kruskal weight:', mst.kruskal.totalWeight, 'Prim weight:', mst.prim.totalWeight, 'Weights match:', mst.weightsMatch);
      if (!mst.weightsMatch) throw new Error('MST test failed');

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
