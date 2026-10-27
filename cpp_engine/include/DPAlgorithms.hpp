#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include "Course.hpp"
#include "json.hpp"

using json = nlohmann::json;

class DPAlgorithms {
public:
    // 1. 0/1 Knapsack: Optimal Elective Selection under Credit Budget
    static json knapsackElectiveOptimization(
        const std::vector<Course>& electives,
        const std::vector<double>& benefitValues,
        int creditBudget
    ) {
        int n = static_cast<int>(electives.size());
        if (n == 0 || creditBudget <= 0) {
            return json{
                {"maxBenefit", 0},
                {"creditsUsed", 0},
                {"budget", creditBudget},
                {"selectedCourses", json::array()},
                {"explanation", "No courses available or credit budget is zero."}
            };
        }

        // dp[i][w] = maximum benefit using subset of first i courses with weight limit w
        // Convert benefits to integers (e.g. * 10) for exact discrete DP or use double DP
        std::vector<std::vector<double>> dp(n + 1, std::vector<double>(creditBudget + 1, 0.0));

        for (int i = 1; i <= n; ++i) {
            int wt = electives[i - 1].credits;
            double val = benefitValues[i - 1];

            for (int w = 0; w <= creditBudget; ++w) {
                if (wt <= w) {
                    dp[i][w] = std::max(dp[i - 1][w], dp[i - 1][w - wt] + val);
                } else {
                    dp[i][w] = dp[i - 1][w];
                }
            }
        }

        // Backtrack to find selected items
        std::vector<Course> selected;
        int currW = creditBudget;
        for (int i = n; i > 0; --i) {
            if (std::abs(dp[i][currW] - dp[i - 1][currW]) > 1e-4) {
                selected.push_back(electives[i - 1]);
                currW -= electives[i - 1].credits;
            }
        }
        std::reverse(selected.begin(), selected.end());

        int totalCreditsUsed = 0;
        json selectedJson = json::array();
        for (const auto& c : selected) {
            totalCreditsUsed += c.credits;
            selectedJson.push_back(c.toJson());
        }

        // Generate DP table snapshot for UI visualization
        json tableJson = json::array();
        for (int i = 0; i <= n; ++i) {
            json row = json::array();
            for (int w = 0; w <= creditBudget; ++w) {
                row.push_back(std::round(dp[i][w] * 10.0) / 10.0);
            }
            tableJson.push_back(row);
        }

        return json{
            {"algorithm", "0/1 Knapsack Dynamic Programming"},
            {"description", "Finds the combination of elective courses that maximizes academic benefit without exceeding the credit budget."},
            {"timeComplexity", "O(N * W)"},
            {"spaceComplexity", "O(N * W)"},
            {"maxBenefit", std::round(dp[n][creditBudget] * 100.0) / 100.0},
            {"creditsUsed", totalCreditsUsed},
            {"creditBudget", creditBudget},
            {"selectedCourses", selectedJson},
            {"dpTable", tableJson}
        };
    }

    // 2. Longest Common Subsequence (LCS): Syllabus & Skill Alignment
    static json longestCommonSubsequence(const std::string& str1, const std::string& str2) {
        int m = static_cast<int>(str1.size());
        int n = static_cast<int>(str2.size());

        std::vector<std::vector<int>> dp(m + 1, std::vector<int>(n + 1, 0));

        for (int i = 1; i <= m; ++i) {
            for (int j = 1; j <= n; ++j) {
                if (std::tolower(str1[i - 1]) == std::tolower(str2[j - 1])) {
                    dp[i][j] = dp[i - 1][j - 1] + 1;
                } else {
                    dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
                }
            }
        }

        // Backtrack to find LCS string
        std::string lcsStr = "";
        int i = m, j = n;
        while (i > 0 && j > 0) {
            if (std::tolower(str1[i - 1]) == std::tolower(str2[j - 1])) {
                lcsStr.push_back(str1[i - 1]);
                i--;
                j--;
            } else if (dp[i - 1][j] > dp[i][j - 1]) {
                i--;
            } else {
                j--;
            }
        }
        std::reverse(lcsStr.begin(), lcsStr.end());

        // Subsequence match similarity ratio
        double similarity = 0.0;
        if (std::max(m, n) > 0) {
            similarity = (2.0 * dp[m][n]) / (m + n);
        }

        json dpTable = json::array();
        // Return truncated or full table if not excessive
        int limitM = std::min(m, 25);
        int limitN = std::min(n, 25);
        for (int r = 0; r <= limitM; ++r) {
            json row = json::array();
            for (int c = 0; c <= limitN; ++c) {
                row.push_back(dp[r][c]);
            }
            dpTable.push_back(row);
        }

        return json{
            {"algorithm", "Longest Common Subsequence (LCS)"},
            {"description", "Measures structural topic and prerequisite overlap between student goals and course content."},
            {"timeComplexity", "O(M * N)"},
            {"spaceComplexity", "O(M * N)"},
            {"input1", str1},
            {"input2", str2},
            {"lcsLength", dp[m][n]},
            {"lcsString", lcsStr},
            {"similarityRatio", std::round(similarity * 1000.0) / 1000.0},
            {"dpTablePreview", dpTable}
        };
    }

    // 3. Resource Allocation DP: Multi-stage Study/Lab Time Distribution
    static json resourceAllocationDP(
        const std::vector<std::string>& courseNames,
        const std::vector<int>& difficulties,
        int totalHours
    ) {
        int n = static_cast<int>(courseNames.size());
        if (n == 0 || totalHours <= 0) {
            return json{
                {"maxGain", 0.0},
                {"totalHours", totalHours},
                {"allocations", json::array()},
                {"explanation", "No courses or hours specified."}
            };
        }

        // Gain function: g(course i, hours h) = difficulty_weight * log2(1 + h) * 10
        // Diminishing returns curve common in cognitive learning and project resource scheduling
        std::vector<std::vector<double>> gain(n, std::vector<double>(totalHours + 1, 0.0));
        for (int i = 0; i < n; ++i) {
            double diffWeight = 1.0 + (difficulties[i] * 0.4);
            for (int h = 0; h <= totalHours; ++h) {
                gain[i][h] = diffWeight * std::log2(1.0 + h) * 12.0;
            }
        }

        // dp[i][h] = max gain allocating h hours across first i courses
        std::vector<std::vector<double>> dp(n, std::vector<double>(totalHours + 1, 0.0));
        std::vector<std::vector<int>> choice(n, std::vector<int>(totalHours + 1, 0));

        // Base case: first course
        for (int h = 0; h <= totalHours; ++h) {
            dp[0][h] = gain[0][h];
            choice[0][h] = h;
        }

        // Multi-stage optimization
        for (int i = 1; i < n; ++i) {
            for (int h = 0; h <= totalHours; ++h) {
                double best = -1.0;
                int bestK = 0;
                for (int k = 0; k <= h; ++k) {
                    double val = dp[i - 1][h - k] + gain[i][k];
                    if (val > best) {
                        best = val;
                        bestK = k;
                    }
                }
                dp[i][h] = best;
                choice[i][h] = bestK;
            }
        }

        // Backtrack optimal allocations
        std::vector<int> allocated(n, 0);
        int remHours = totalHours;
        for (int i = n - 1; i >= 0; --i) {
            allocated[i] = choice[i][remHours];
            remHours -= allocated[i];
        }

        json allocJson = json::array();
        for (int i = 0; i < n; ++i) {
            allocJson.push_back({
                {"course", courseNames[i]},
                {"difficulty", difficulties[i]},
                {"hoursAllocated", allocated[i]},
                {"gainScore", std::round(gain[i][allocated[i]] * 10.0) / 10.0}
            });
        }

        return json{
            {"algorithm", "Multi-Stage Dynamic Programming (Resource Allocation)"},
            {"description", "Optimizes weekly study time/GPU cluster quota distribution across rigorous courses using principle of optimality under diminishing marginal returns."},
            {"timeComplexity", "O(N * H^2)"},
            {"spaceComplexity", "O(N * H)"},
            {"totalHoursAvailable", totalHours},
            {"maxKnowledgeGain", std::round(dp[n - 1][totalHours] * 100.0) / 100.0},
            {"allocations", allocJson}
        };
    }
};
