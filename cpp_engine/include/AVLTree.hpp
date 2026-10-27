#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Course.hpp"
#include "json.hpp"

using json = nlohmann::json;

struct AVLNode {
    Course course;
    int height;
    AVLNode* left;
    AVLNode* right;

    AVLNode(const Course& c) : course(c), height(1), left(nullptr), right(nullptr) {}
};

class AVLTree {
private:
    AVLNode* root;
    size_t count;

    int getHeight(AVLNode* node) const {
        return node ? node->height : 0;
    }

    int getBalanceFactor(AVLNode* node) const {
        return node ? getHeight(node->left) - getHeight(node->right) : 0;
    }

    void updateHeight(AVLNode* node) {
        if (node) {
            node->height = 1 + std::max(getHeight(node->left), getHeight(node->right));
        }
    }

    // Right rotation (LL case)
    AVLNode* rotateRight(AVLNode* y) {
        AVLNode* x = y->left;
        AVLNode* T2 = x->right;

        x->right = y;
        y->left = T2;

        updateHeight(y);
        updateHeight(x);

        return x;
    }

    // Left rotation (RR case)
    AVLNode* rotateLeft(AVLNode* x) {
        AVLNode* y = x->right;
        AVLNode* T2 = y->left;

        y->left = x;
        x->right = T2;

        updateHeight(x);
        updateHeight(y);

        return y;
    }

    AVLNode* insert(AVLNode* node, const Course& course, bool& inserted) {
        if (!node) {
            inserted = true;
            count++;
            return new AVLNode(course);
        }

        if (course.id < node->course.id) {
            node->left = insert(node->left, course, inserted);
        } else if (course.id > node->course.id) {
            node->right = insert(node->right, course, inserted);
        } else {
            // Duplicate ID: update course content
            node->course = course;
            inserted = false;
            return node;
        }

        updateHeight(node);
        int balance = getBalanceFactor(node);

        // Case 1: Left Left
        if (balance > 1 && course.id < node->left->course.id) {
            return rotateRight(node);
        }

        // Case 2: Right Right
        if (balance < -1 && course.id > node->right->course.id) {
            return rotateLeft(node);
        }

        // Case 3: Left Right
        if (balance > 1 && course.id > node->left->course.id) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }

        // Case 4: Right Left
        if (balance < -1 && course.id < node->right->course.id) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }

        return node;
    }

    AVLNode* search(AVLNode* node, const std::string& id) const {
        if (!node || node->course.id == id) {
            return node;
        }
        if (id < node->course.id) {
            return search(node->left, id);
        }
        return search(node->right, id);
    }

    void inorder(AVLNode* node, std::vector<Course>& result) const {
        if (!node) return;
        inorder(node->left, result);
        result.push_back(node->course);
        inorder(node->right, result);
    }

    void preorder(AVLNode* node, std::vector<Course>& result) const {
        if (!node) return;
        result.push_back(node->course);
        preorder(node->left, result);
        preorder(node->right, result);
    }

    void postorder(AVLNode* node, std::vector<Course>& result) const {
        if (!node) return;
        postorder(node->left, result);
        postorder(node->right, result);
        result.push_back(node->course);
    }

    void rangeSearch(AVLNode* node, const std::string& low, const std::string& high, std::vector<Course>& result) const {
        if (!node) return;
        if (node->course.id > low) {
            rangeSearch(node->left, low, high, result);
        }
        if (node->course.id >= low && node->course.id <= high) {
            result.push_back(node->course);
        }
        if (node->course.id < high) {
            rangeSearch(node->right, low, high, result);
        }
    }

    json serializeNode(AVLNode* node) const {
        if (!node) return nullptr;
        json j;
        j["id"] = node->course.id;
        j["name"] = node->course.name;
        j["credits"] = node->course.credits;
        j["difficulty"] = node->course.difficulty;
        j["track"] = node->course.track;
        j["height"] = node->height;
        j["balanceFactor"] = getBalanceFactor(node);
        j["left"] = serializeNode(node->left);
        j["right"] = serializeNode(node->right);
        return j;
    }

    void destroy(AVLNode* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

public:
    AVLTree() : root(nullptr), count(0) {}

    ~AVLTree() {
        destroy(root);
    }

    // Disable copy for simplicity, allow move
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;
    AVLTree(AVLTree&& other) noexcept : root(other.root), count(other.count) {
        other.root = nullptr;
        other.count = 0;
    }

    bool insert(const Course& course) {
        bool inserted = false;
        root = insert(root, course, inserted);
        return inserted;
    }

    const Course* search(const std::string& id) const {
        AVLNode* res = search(root, id);
        return res ? &(res->course) : nullptr;
    }

    std::vector<Course> getInOrder() const {
        std::vector<Course> result;
        inorder(root, result);
        return result;
    }

    std::vector<Course> getPreOrder() const {
        std::vector<Course> result;
        preorder(root, result);
        return result;
    }

    std::vector<Course> getPostOrder() const {
        std::vector<Course> result;
        postorder(root, result);
        return result;
    }

    std::vector<Course> getRange(const std::string& low, const std::string& high) const {
        std::vector<Course> result;
        rangeSearch(root, low, high, result);
        return result;
    }

    size_t size() const { return count; }
    int getHeight() const { return getHeight(root); }
    bool isEmpty() const { return root == nullptr; }

    json toVisualJson() const {
        return json{
            {"root", serializeNode(root)},
            {"totalNodes", count},
            {"treeHeight", getHeight(root)}
        };
    }
};
