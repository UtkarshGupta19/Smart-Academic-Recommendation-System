const express = require('express');
const cors = require('cors');
const path = require('path');
const fs = require('fs');
const { spawn } = require('child_process');

const app = express();
const PORT = process.env.PORT || 3000;
const HOST = process.env.HOST || '0.0.0.0';

app.use(cors());
app.use(express.json());
app.use(express.static(path.join(__dirname, '../public')));

// Path to C++ DSA Engine executable (configurable via environment variable)
const ENGINE_PATH = process.env.ENGINE_PATH || path.resolve(__dirname, '../cpp_engine/bin/sars_engine');

// Load initial sample data
const coursesFilePath = path.join(__dirname, 'data/courses.json');
const studentsFilePath = path.join(__dirname, 'data/students.json');

let coursesData = JSON.parse(fs.readFileSync(coursesFilePath, 'utf8'));
let studentsData = JSON.parse(fs.readFileSync(studentsFilePath, 'utf8'));

// Helper: build edges from prerequisite relationships
function buildGraphEdges(courses) {
  const edges = [];
  courses.forEach(c => {
    if (Array.isArray(c.prerequisites)) {
      c.prerequisites.forEach(pre => {
        // Edge: pre -> c.id with effort weight proportional to difficulty and credits
        edges.push({
          from: pre,
          to: c.id,
          weight: Math.round((c.difficulty * 1.5 + (c.credits * 0.5)) * 10) / 10,
          label: 'Prerequisite'
        });
      });
    }
  });
  return edges;
}

// Safe execution of C++ Engine via child_process.spawn with stdin
function runCppEngine(command, data = {}) {
  return new Promise((resolve, reject) => {
    if (!fs.existsSync(ENGINE_PATH)) {
      return reject(new Error(`C++ Engine binary not found at ${ENGINE_PATH}. Please compile it using cpp_engine/build.sh.`));
    }

    const payload = JSON.stringify({ command, data });
    const proc = spawn(ENGINE_PATH, [], {
      stdio: ['pipe', 'pipe', 'pipe']
    });

    let stdoutData = '';
    let stderrData = '';

    proc.stdout.on('data', chunk => {
      stdoutData += chunk.toString();
    });

    proc.stderr.on('data', chunk => {
      stderrData += chunk.toString();
    });

    proc.on('close', code => {
      if (code !== 0) {
        return reject(new Error(`C++ Engine exited with code ${code}. Error: ${stderrData || stdoutData}`));
      }
      try {
        const jsonResult = JSON.parse(stdoutData.trim());
        if (jsonResult.status === 'error') {
          return reject(new Error(jsonResult.error || 'Engine reported an error'));
        }
        resolve(jsonResult.result);
      } catch (err) {
        reject(new Error(`Failed to parse C++ Engine output: ${err.message}. Raw output: ${stdoutData}`));
      }
    });

    proc.on('error', err => {
      reject(new Error(`Failed to spawn C++ Engine process: ${err.message}`));
    });

    // Write input payload safely to stdin
    proc.stdin.write(payload);
    proc.stdin.end();
  });
}

// ---------------- API ENDPOINTS ---------------- //

// Health check
app.get('/api/health', (req, res) => {
  res.json({
    status: 'online',
    engine: fs.existsSync(ENGINE_PATH) ? 'compiled' : 'missing',
    totalCourses: coursesData.length,
    totalStudents: studentsData.length,
    timestamp: new Date().toISOString()
  });
});

// 1. Course Catalog via AVL Tree
app.get('/api/courses', async (req, res) => {
  try {
    const result = await runCppEngine('catalog_inorder', { courses: coursesData });
    res.json(result);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// Search course in AVL Tree
app.get('/api/courses/search/:id', async (req, res) => {
  try {
    const { id } = req.params;
    const result = await runCppEngine('catalog_search', {
      courses: coursesData,
      id: id.toUpperCase().trim()
    });
    res.json(result);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// AVL Tree Range Search
app.get('/api/courses/range', async (req, res) => {
  try {
    const low = (req.query.low || 'CS100').toUpperCase();
    const high = (req.query.high || 'CS999').toUpperCase();
    const result = await runCppEngine('catalog_range', {
      courses: coursesData,
      low,
      high
    });
    res.json(result);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// AVL Tree Visual Structure
app.get('/api/avl-visual', async (req, res) => {
  try {
    const result = await runCppEngine('avl_visual', { courses: coursesData });
    res.json(result);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 2. Student Profiles
app.get('/api/students', (req, res) => {
  res.json(studentsData);
});

// Update or add student profile
app.post('/api/students', (req, res) => {
  const profile = req.body;
  if (!profile.id || !profile.name) {
    return res.status(400).json({ error: 'Profile must contain id and name' });
  }
  const idx = studentsData.findIndex(s => s.id === profile.id);
  if (idx >= 0) {
    studentsData[idx] = { ...studentsData[idx], ...profile };
  } else {
    studentsData.push(profile);
  }
  res.json({ status: 'success', profile: studentsData.find(s => s.id === profile.id) });
});

// 3. Course Recommendations via C++ Max-Heap
app.post('/api/recommend', async (req, res) => {
  try {
    const {
      completedCourses = [],
      interests = [],
      gpa = 3.5,
      creditBudget = 15,
      topK = 8
    } = req.body;

    const result = await runCppEngine('recommend', {
      courses: coursesData,
      completedCourses,
      interests,
      gpa: Number(gpa),
      creditBudget: Number(creditBudget),
      topK: Number(topK)
    });
    res.json(result);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 4. Learning Paths: BFS (fewest hops) vs Dijkstra (minimum effort)
app.post('/api/learning-path', async (req, res) => {
  try {
    const { start, target } = req.body;
    if (!start || !target) {
      return res.status(400).json({ error: 'Both start and target course IDs are required.' });
    }
    const edges = buildGraphEdges(coursesData);
    const result = await runCppEngine('learning_path', {
      courses: coursesData,
      edges,
      start: start.toUpperCase().trim(),
      target: target.toUpperCase().trim()
    });
    res.json(result);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 5. Elective Optimization (0/1 Knapsack)
app.post('/api/knapsack', async (req, res) => {
  try {
    const {
      electives = coursesData.filter(c => c.track !== 'Core'),
      creditBudget = 12
    } = req.body;

    const result = await runCppEngine('knapsack', {
      electives,
      creditBudget: Number(creditBudget)
    });
    res.json(result);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 6. Longest Common Subsequence (LCS)
app.post('/api/lcs', async (req, res) => {
  try {
    const { str1, str2 } = req.body;
    if (typeof str1 !== 'string' || typeof str2 !== 'string') {
      return res.status(400).json({ error: 'str1 and str2 must be strings.' });
    }
    const result = await runCppEngine('lcs', { str1, str2 });
    res.json(result);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 7. Resource Allocation DP
app.post('/api/resource-allocation', async (req, res) => {
  try {
    const {
      courses = coursesData.slice(0, 5),
      totalHours = 20
    } = req.body;

    const result = await runCppEngine('resource_allocation', {
      courses,
      totalHours: Number(totalHours)
    });
    res.json(result);
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 8. Minimum Spanning Tree: Prim vs Kruskal
app.get('/api/mst', async (req, res) => {
  try {
    // Generate connected academic network
    const vertices = coursesData.map(c => c.id);
    const edges = [];
    // Include prerequisite edges
    coursesData.forEach(c => {
      c.prerequisites.forEach(pre => {
        edges.push({
          u: pre,
          v: c.id,
          weight: Math.round((c.difficulty * 2.0 + c.credits) * 10) / 10,
          label: 'Skill Link'
        });
      });
    });

    // Add inter-track bridge edges to ensure full graph connectivity
    const tracks = ['Core', 'AI/ML', 'Systems', 'Web', 'Cybersecurity', 'Theory'];
    const trackLeaders = {
      'Core': 'CS101',
      'Theory': 'CS102',
      'Systems': 'CS202',
      'AI/ML': 'CS401',
      'Web': 'CS404',
      'Cybersecurity': 'CS406'
    };

    edges.push({ u: 'CS101', v: 'CS102', weight: 3.5, label: 'Interdisciplinary' });
    edges.push({ u: 'CS201', v: 'CS202', weight: 4.0, label: 'Interdisciplinary' });
    edges.push({ u: 'CS301', v: 'CS401', weight: 5.5, label: 'Interdisciplinary' });
    edges.push({ u: 'CS302', v: 'CS405', weight: 4.5, label: 'Interdisciplinary' });
    edges.push({ u: 'CS304', v: 'CS406', weight: 4.2, label: 'Interdisciplinary' });
    edges.push({ u: 'CS101', v: 'CS404', weight: 3.0, label: 'Interdisciplinary' });
    edges.push({ u: 'CS303', v: 'CS404', weight: 3.8, label: 'Interdisciplinary' });

    const result = await runCppEngine('mst_compare', {
      vertices,
      edges
    });
    res.json({ ...result, networkNodes: vertices, originalEdgeCount: edges.length });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// 9. Comprehensive Graph Algorithms Lab (DFS, Bellman-Ford, Floyd-Warshall, Connected Components)
app.get('/api/graph/algorithms', async (req, res) => {
  try {
    const edges = buildGraphEdges(coursesData);
    const startNode = req.query.start || 'CS101';

    const [dfs, bellmanFord, connected] = await Promise.all([
      runCppEngine('graph_dfs', { courses: coursesData, edges, start: startNode }),
      runCppEngine('graph_bellman_ford', { courses: coursesData, edges, start: startNode }),
      runCppEngine('graph_connected', { courses: coursesData, edges })
    ]);

    res.json({
      dfs,
      bellmanFord,
      connected,
      graphVisual: {
        nodes: coursesData,
        edges
      }
    });
  } catch (err) {
    res.status(500).json({ error: err.message });
  }
});

// Fallback to index.html for SPA routing
app.get('*', (req, res) => {
  res.sendFile(path.join(__dirname, '../public/index.html'));
});

// Start Express Server
if (require.main === module) {
  app.listen(PORT, HOST, () => {
    console.log(`SARS API Server running smoothly on http://${HOST}:${PORT}`);
    console.log(`C++ DSA Engine connected at: ${ENGINE_PATH}`);
  });
}

module.exports = app;
