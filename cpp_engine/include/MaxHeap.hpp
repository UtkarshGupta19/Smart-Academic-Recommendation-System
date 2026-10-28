#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <cmath>
#include "Course.hpp"
#include "json.hpp"

using json = nlohmann::json;

struct RecommendationItem {
    Course course;
    double totalScore;
    double trackScore;
    double interestScore;    // Keywords overlap
    double gpaFitScore;
    double prereqScore;
    double budgetScore;
    bool isEligible;
    std::vector<std::string> missingPrerequisites;
    std::string rationale;
    std::vector<std::string> matchedKeywords;

    bool operator<(const RecommendationItem& other) const {
        return totalScore < other.totalScore;
    }

    bool operator>(const RecommendationItem& other) const {
        return totalScore > other.totalScore;
    }

    json toJson() const {
        return json{
            {"course", course.toJson()},
            {"totalScore", std::round(totalScore * 10.0) / 10.0},
            {"trackScore", std::round(trackScore * 10.0) / 10.0},
            {"interestScore", std::round(interestScore * 10.0) / 10.0},
            {"gpaFitScore", std::round(gpaFitScore * 10.0) / 10.0},
            {"prereqScore", std::round(prereqScore * 10.0) / 10.0},
            {"budgetScore", std::round(budgetScore * 10.0) / 10.0},
            {"isEligible", isEligible},
            {"missingPrerequisites", missingPrerequisites},
            {"rationale", rationale},
            {"matchedKeywords", matchedKeywords}
        };
    }
};

// Custom Binary Max-Heap implementation from scratch
class MaxHeap {
private:
    std::vector<RecommendationItem> heap;

    int parent(int i) const { return (i - 1) / 2; }
    int left(int i) const { return 2 * i + 1; }
    int right(int i) const { return 2 * i + 2; }

    void heapifyUp(int i) {
        while (i > 0 && heap[parent(i)] < heap[i]) {
            std::swap(heap[parent(i)], heap[i]);
            i = parent(i);
        }
    }

    void heapifyDown(int i) {
        int maxIndex = i;
        int l = left(i);
        int r = right(i);
        int n = static_cast<int>(heap.size());

        if (l < n && heap[l] > heap[maxIndex]) {
            maxIndex = l;
        }
        if (r < n && heap[r] > heap[maxIndex]) {
            maxIndex = r;
        }

        if (i != maxIndex) {
            std::swap(heap[i], heap[maxIndex]);
            heapifyDown(maxIndex);
        }
    }

public:
    MaxHeap() = default;

    void push(const RecommendationItem& item) {
        heap.push_back(item);
        heapifyUp(static_cast<int>(heap.size()) - 1);
    }

    RecommendationItem pop() {
        if (heap.empty()) {
            throw std::runtime_error("Heap is empty");
        }
        RecommendationItem root = heap.front();
        heap.front() = heap.back();
        heap.pop_back();
        if (!heap.empty()) {
            heapifyDown(0);
        }
        return root;
    }

    const RecommendationItem& top() const {
        if (heap.empty()) {
            throw std::runtime_error("Heap is empty");
        }
        return heap.front();
    }

    size_t size() const { return heap.size(); }
    bool empty() const { return heap.empty(); }
    void clear() { heap.clear(); }

    const std::vector<RecommendationItem>& getRawData() const {
        return heap;
    }
};

// Course Recommender Engine using Max-Heap
class CourseRecommender {
public:
    static json rankCourses(
        const std::vector<Course>& catalog,
        const std::unordered_set<std::string>& completedCourses,
        const std::vector<std::string>& interests,
        double gpa, // scale 0.0 - 4.0 or 0.0 - 10.0
        int creditBudget,
        int topK = 10
    ) {
        // Normalize GPA to 4.0 scale if provided on 10.0 scale
        double normGpa = (gpa > 4.5) ? (gpa / 2.5) : gpa;
        normGpa = std::max(0.0, std::min(4.0, normGpa));

        MaxHeap heap;
        std::vector<RecommendationItem> ineligibleList;

        for (const auto& course : catalog) {
            // Skip already completed courses
            if (completedCourses.find(course.id) != completedCourses.end()) {
                continue;
            }

            // Check prerequisites
            std::vector<std::string> missing;
            for (const auto& req : course.prerequisites) {
                if (completedCourses.find(req) == completedCourses.end()) {
                    missing.push_back(req);
                }
            }
            bool isEligible = missing.empty();

            // 1. Interest Score & Track Matching (up to 50 pts total)
            double trackScore = 0.0;
            double interestScore = 0.0;
            std::vector<std::string> matchedKeywords;

            for (const auto& interest : interests) {
                if (course.track == interest) {
                    trackScore = std::max(trackScore, 30.0);
                }
                for (const auto& kw : course.keywords) {
                    if (kw == interest || interest.find(kw) != std::string::npos || kw.find(interest) != std::string::npos) {
                        interestScore += 5.0;
                        matchedKeywords.push_back(kw);
                    }
                }
            }
            interestScore = std::min(interestScore, 20.0);

            // 2. GPA Rigor Fit (up to 25 pts)
            // High GPA (>=3.5) rewards challenging courses (difficulty 4-5)
            // Medium GPA rewards moderate courses (difficulty 3)
            // Developing GPA rewards solid foundational courses (difficulty 1-2)
            double gpaFitScore = 0.0;
            if (normGpa >= 3.5) {
                gpaFitScore = 15.0 + (course.difficulty * 2.0);
            } else if (normGpa >= 2.8) {
                gpaFitScore = 20.0 - std::abs(course.difficulty - 3) * 4.0;
            } else {
                gpaFitScore = 20.0 - (course.difficulty * 3.0);
            }
            gpaFitScore = std::max(5.0, gpaFitScore);

            // 3. Prerequisite Preparedness Bonus (15 pts if met, penalized if missing)
            double prereqScore = 0.0;
            if (isEligible) {
                prereqScore = 15.0; // Ready to enroll immediately
            } else {
                prereqScore = -15.0 * static_cast<double>(missing.size());
            }

            // 4. Budget fit bonus (+10 pts if within credit limit, -10 pts if exceeds)
            double budgetScore = 0.0;
            if (course.credits <= creditBudget) {
                budgetScore = 10.0;
            } else {
                budgetScore = -10.0; // Exceeds current semester credit budget
            }

            // Exact mathematical sum of all components
            double totalScore = trackScore + interestScore + gpaFitScore + prereqScore + budgetScore;

            // Detailed rationale breakdown
            std::string rationale = "";
            if (!isEligible) {
                rationale = "Prerequisites incomplete (" + std::to_string(missing.size()) + " pending: ";
                for (size_t i = 0; i < missing.size(); ++i) {
                    if (i > 0) rationale += ", ";
                    rationale += missing[i];
                }
                rationale += ").";
            } else {
                std::vector<std::string> parts;
                if (trackScore > 0) {
                    parts.push_back("Track alignment with " + course.track + " (+" + std::to_string(static_cast<int>(trackScore)) + " pts)");
                }
                if (interestScore > 0 && !matchedKeywords.empty()) {
                    std::string kws = "Keywords matched: ";
                    for (size_t i = 0; i < matchedKeywords.size(); ++i) {
                        if (i > 0) kws += ", ";
                        kws += matchedKeywords[i];
                    }
                    kws += " (+" + std::to_string(static_cast<int>(interestScore)) + " pts)";
                    parts.push_back(kws);
                }
                parts.push_back("Difficulty " + std::to_string(course.difficulty) + "/5 matched to GPA " + std::to_string(normGpa).substr(0, 4) + " (+" + std::to_string(static_cast<int>(gpaFitScore)) + " pts)");
                parts.push_back("Prerequisites completed (+15 pts)");
                if (budgetScore > 0) {
                    parts.push_back(std::to_string(course.credits) + " cr fits in budget of " + std::to_string(creditBudget) + " cr (+10 pts)");
                } else {
                    parts.push_back(std::to_string(course.credits) + " cr exceeds budget of " + std::to_string(creditBudget) + " cr (-10 pts)");
                }
                for (size_t i = 0; i < parts.size(); ++i) {
                    if (i > 0) rationale += " • ";
                    rationale += parts[i];
                }
            }

            RecommendationItem item{
                course,
                totalScore,
                trackScore,
                interestScore,
                gpaFitScore,
                prereqScore,
                budgetScore,
                isEligible,
                missing,
                rationale,
                matchedKeywords
            };

            if (isEligible) {
                heap.push(item);
            } else {
                ineligibleList.push_back(item);
            }
        }

        // Snapshot initial Max-Heap array structure before extractions
        json initialHeapArray = json::array();
        const auto& rawData = heap.getRawData();
        for (size_t i = 0; i < rawData.size(); ++i) {
            json nodeInfo = {
                {"index", i},
                {"courseId", rawData[i].course.id},
                {"courseName", rawData[i].course.name},
                {"score", std::round(rawData[i].totalScore * 10.0) / 10.0}
            };
            if (i > 0) nodeInfo["parentIndex"] = (i - 1) / 2;
            if (2 * i + 1 < rawData.size()) nodeInfo["leftChildIndex"] = 2 * i + 1;
            if (2 * i + 2 < rawData.size()) nodeInfo["rightChildIndex"] = 2 * i + 2;
            initialHeapArray.push_back(nodeInfo);
        }

        // Extract top-K from Max-Heap and log each heapify step
        json recommendations = json::array();
        json extractionLog = json::array();
        int extracted = 0;
        while (!heap.empty() && extracted < topK) {
            size_t prevSize = heap.size();
            RecommendationItem topItem = heap.pop();
            recommendations.push_back(topItem.toJson());
            extracted++;

            extractionLog.push_back({
                {"step", extracted},
                {"poppedCourse", topItem.course.id},
                {"score", std::round(topItem.totalScore * 10.0) / 10.0},
                {"heapSizeBefore", prevSize},
                {"heapSizeAfter", heap.size()},
                {"operation", "Root extracted in O(1); back element moved to root and heapifyDown(0) restored Max-Heap property in O(log N)."}
            });
        }

        // Sort ineligible by totalScore descending
        std::sort(ineligibleList.begin(), ineligibleList.end(), std::greater<RecommendationItem>());
        json lockedRecommendations = json::array();
        for (size_t i = 0; i < std::min(ineligibleList.size(), size_t(5)); ++i) {
            lockedRecommendations.push_back(ineligibleList[i].toJson());
        }

        return json{
            {"algorithm", "Max-Heap Priority Ranking"},
            {"description", "Extracts optimal course recommendations in O(K log N) time based on multi-criteria heuristic scoring."},
            {"recommendations", recommendations},
            {"lockedCourses", lockedRecommendations},
            {"totalEligible", extracted + heap.size()},
            {"initialHeapArray", initialHeapArray},
            {"extractionLog", extractionLog}
        };
    }
};
