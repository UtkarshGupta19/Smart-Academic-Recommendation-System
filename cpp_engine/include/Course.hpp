#pragma once

#include <string>
#include <vector>
#include "json.hpp"

using json = nlohmann::json;

struct Course {
    std::string id;
    std::string name;
    std::string track;            // e.g. "Core", "AI/ML", "Systems", "Web", "Cybersecurity", "Theory"
    int credits = 3;              // 1 to 5
    int difficulty = 3;           // 1 (Beginner) to 5 (Advanced)
    int semester = 1;             // 1 to 8
    std::vector<std::string> prerequisites;
    std::vector<std::string> keywords;
    std::string description;

    // JSON serialization
    json toJson() const {
        return json{
            {"id", id},
            {"name", name},
            {"track", track},
            {"credits", credits},
            {"difficulty", difficulty},
            {"semester", semester},
            {"prerequisites", prerequisites},
            {"keywords", keywords},
            {"description", description}
        };
    }

    static Course fromJson(const json& j) {
        Course c;
        if (j.contains("id")) c.id = j["id"].get<std::string>();
        if (j.contains("name")) c.name = j["name"].get<std::string>();
        if (j.contains("track")) c.track = j["track"].get<std::string>();
        if (j.contains("credits")) c.credits = j["credits"].get<int>();
        if (j.contains("difficulty")) c.difficulty = j["difficulty"].get<int>();
        if (j.contains("semester")) c.semester = j["semester"].get<int>();
        if (j.contains("prerequisites")) c.prerequisites = j["prerequisites"].get<std::vector<std::string>>();
        if (j.contains("keywords")) c.keywords = j["keywords"].get<std::vector<std::string>>();
        if (j.contains("description")) c.description = j["description"].get<std::string>();
        return c;
    }
};
