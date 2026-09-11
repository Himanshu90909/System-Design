#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace netflix {

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;
using Seconds = std::chrono::seconds;

enum class ContentType { Movie, Series };

struct Content {
    std::string id;
    std::string title;
    ContentType type;
    std::string genre;
    std::vector<std::string> keywords;
    int durationSeconds;
    bool published{true};
};

struct Profile {
    std::string id;
    std::string name;
    std::unordered_set<std::string> watchlist;
    std::unordered_map<std::string, int> progressSeconds;
    std::unordered_map<std::string, int> genreViews;
};

struct StreamSession {
    std::string sessionId;
    std::string contentId;
    std::string profileId;
    std::string deviceType;
    std::string streamUrl;
    TimePoint expiresAt;
};

class StreamingPlatform {
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Content> catalog_;
    std::unordered_map<std::string, Profile> profiles_;
    std::unordered_map<std::string, StreamSession> sessions_;
    std::uint64_t nextSession_{1};

    Content& content(const std::string& contentId) {
        auto it = catalog_.find(contentId);
        if (it == catalog_.end() || !it->second.published) throw std::out_of_range("content not found");
        return it->second;
    }

    Profile& profile(const std::string& profileId) {
        auto it = profiles_.find(profileId);
        if (it == profiles_.end()) throw std::out_of_range("profile not found");
        return it->second;
    }

    static bool containsIgnoreCase(const std::string& value, const std::string& query) {
        auto normalize = [](std::string input) {
            std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
            return input;
        };
        return normalize(value).find(normalize(query)) != std::string::npos;
    }

public:
    void addProfile(std::string profileId, std::string name) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (profiles_.find(profileId) != profiles_.end()) throw std::invalid_argument("profile already exists");
        const auto key = profileId;
        profiles_.emplace(key, Profile{std::move(profileId), std::move(name), {}, {}, {}});
    }

    void publish(Content item) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (item.id.empty() || item.title.empty() || item.durationSeconds <= 0) throw std::invalid_argument("invalid content");
        catalog_[item.id] = std::move(item);
    }

    std::vector<Content> search(const std::string& query) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Content> result;
        for (const auto& entry : catalog_) {
            const auto& item = entry.second;
            if (!item.published) continue;
            bool matched = containsIgnoreCase(item.title, query) || containsIgnoreCase(item.genre, query);
            for (const auto& keyword : item.keywords) matched = matched || containsIgnoreCase(keyword, query);
            if (matched) result.push_back(item);
        }
        std::sort(result.begin(), result.end(), [](const Content& left, const Content& right) { return left.title < right.title; });
        return result;
    }

    void addToWatchlist(const std::string& profileId, const std::string& contentId) {
        std::lock_guard<std::mutex> lock(mutex_);
        static_cast<void>(content(contentId));
        profile(profileId).watchlist.insert(contentId);
    }

    void removeFromWatchlist(const std::string& profileId, const std::string& contentId) {
        std::lock_guard<std::mutex> lock(mutex_);
        profile(profileId).watchlist.erase(contentId);
    }

    StreamSession startStream(const std::string& profileId, const std::string& contentId,
                              std::string deviceType, TimePoint now = Clock::now()) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& item = content(contentId);
        auto& currentProfile = profile(profileId);
        const auto sessionId = "session-" + std::to_string(nextSession_++);
        const auto expiresAt = now + Seconds(3600);
        StreamSession session{sessionId, item.id, currentProfile.id, std::move(deviceType),
                              "https://cdn.example.test/vod/" + item.id + "/master.m3u8?session=" + sessionId, expiresAt};
        sessions_[sessionId] = session;
        return session;
    }

    void updateProgress(const std::string& profileId, const std::string& contentId, int seconds, TimePoint now = Clock::now()) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& item = content(contentId);
        auto& currentProfile = profile(profileId);
        if (seconds < 0 || seconds > item.durationSeconds) throw std::invalid_argument("invalid playback position");
        currentProfile.progressSeconds[contentId] = seconds;
        currentProfile.genreViews[item.genre] += 1;
        static_cast<void>(now); // In production this becomes an event timestamp in the watch-history stream.
    }

    std::vector<Content> recommendations(const std::string& profileId) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto profileIt = profiles_.find(profileId);
        if (profileIt == profiles_.end()) throw std::out_of_range("profile not found");
        const auto& currentProfile = profileIt->second;
        std::string preferredGenre;
        int bestViews = -1;
        for (const auto& entry : currentProfile.genreViews) {
            if (entry.second > bestViews) { preferredGenre = entry.first; bestViews = entry.second; }
        }
        std::vector<Content> result;
        for (const auto& entry : catalog_) {
            if (entry.second.published && entry.second.genre == preferredGenre && currentProfile.watchlist.find(entry.first) == currentProfile.watchlist.end()) result.push_back(entry.second);
        }
        return result;
    }

    bool isSessionValid(const std::string& sessionId, TimePoint now = Clock::now()) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = sessions_.find(sessionId);
        return it != sessions_.end() && now < it->second.expiresAt;
    }
};

} // namespace netflix

int main() {
    using namespace netflix;
    const auto now = Clock::now();
    StreamingPlatform platform;
    platform.addProfile("profile-1", "Himanshu");
    platform.publish({"movie-1", "Distributed Systems", ContentType::Movie, "Technology", {"systems", "architecture"}, 7200});
    platform.publish({"movie-2", "Cloud Reliability", ContentType::Movie, "Technology", {"cloud", "availability"}, 5400});
    platform.publish({"series-1", "Ocean Stories", ContentType::Series, "Documentary", {"nature"}, 3600});

    if (platform.search("architecture").size() != 1) return 1;
    platform.addToWatchlist("profile-1", "movie-2");
    auto session = platform.startStream("profile-1", "movie-1", "Smart TV", now);
    if (!platform.isSessionValid(session.sessionId, now + Seconds(30))) return 1;
    platform.updateProgress("profile-1", "movie-1", 1800, now);
    if (platform.recommendations("profile-1").size() != 1) return 1;

    std::cout << "Netflix streaming checks passed\n";
    std::cout << "Stream URL: " << session.streamUrl << "\n";
}
