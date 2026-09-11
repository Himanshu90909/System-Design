#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cctype>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace twitter {

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;

class SnowflakeGenerator {
    std::uint64_t workerId_;
    std::uint64_t lastMillis_{0};
    std::uint64_t sequence_{0};
    static constexpr std::uint64_t Epoch = 1609459200000ULL;

public:
    explicit SnowflakeGenerator(std::uint64_t workerId) : workerId_(workerId & 0x3FF) {}

    std::uint64_t next(TimePoint now = Clock::now()) {
        const auto millis = static_cast<std::uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
        auto timestamp = millis < Epoch ? 0 : millis - Epoch;
        if (timestamp < lastMillis_) throw std::runtime_error("clock moved backwards");
        if (timestamp == lastMillis_) {
            sequence_ = (sequence_ + 1) & 0xFFF;
            if (sequence_ == 0) ++timestamp;
        } else sequence_ = 0;
        lastMillis_ = timestamp;
        return (timestamp << 22) | (workerId_ << 12) | sequence_;
    }
};

struct User {
    std::uint64_t id;
    std::string handle;
    std::unordered_set<std::uint64_t> following;
};

struct Tweet {
    std::uint64_t id;
    std::uint64_t authorId;
    std::string text;
    TimePoint createdAt;
    std::unordered_set<std::uint64_t> likes;
    std::uint64_t repostOf{0};
};

struct TimelineEntry {
    std::uint64_t tweetId;
    double score;
};

class SocialPlatform {
    mutable std::mutex mutex_;
    SnowflakeGenerator ids_;
    std::unordered_map<std::uint64_t, User> users_;
    std::unordered_map<std::string, std::uint64_t> handles_;
    std::unordered_map<std::uint64_t, Tweet> tweets_;
    std::unordered_map<std::uint64_t, std::vector<std::uint64_t>> tweetsByAuthor_;
    std::uint64_t nextUser_{1};

    User& user(std::uint64_t id) {
        auto it = users_.find(id);
        if (it == users_.end()) throw std::out_of_range("user not found");
        return it->second;
    }

    static bool contains(const std::string& value, std::string query) {
        std::transform(query.begin(), query.end(), query.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        std::string normalized = value;
        std::transform(normalized.begin(), normalized.end(), normalized.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return normalized.find(query) != std::string::npos;
    }

    static double rank(const Tweet& tweet, TimePoint now) {
        const auto ageHours = std::max(0.0, std::chrono::duration<double, std::ratio<3600>>(now - tweet.createdAt).count());
        const auto engagement = static_cast<double>(tweet.likes.size());
        return engagement * 2.0 + 1.0 / (1.0 + ageHours);
    }

public:
    explicit SocialPlatform(std::uint64_t workerId = 1) : ids_(workerId) {}

    std::uint64_t registerUser(std::string handle) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (handle.empty() || handles_.find(handle) != handles_.end()) throw std::invalid_argument("handle unavailable");
        const auto id = nextUser_++;
        handles_[handle] = id;
        users_.emplace(id, User{id, std::move(handle), {}});
        return id;
    }

    void follow(std::uint64_t followerId, std::uint64_t followeeId) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (followerId == followeeId) throw std::invalid_argument("cannot follow self");
        static_cast<void>(user(followeeId));
        user(followerId).following.insert(followeeId);
    }

    void unfollow(std::uint64_t followerId, std::uint64_t followeeId) {
        std::lock_guard<std::mutex> lock(mutex_);
        user(followerId).following.erase(followeeId);
    }

    std::uint64_t post(std::uint64_t authorId, std::string text, TimePoint now = Clock::now(), std::uint64_t repostOf = 0) {
        std::lock_guard<std::mutex> lock(mutex_);
        static_cast<void>(user(authorId));
        if (text.empty() || text.size() > 280) throw std::invalid_argument("tweet must be 1-280 characters");
        if (repostOf != 0 && tweets_.find(repostOf) == tweets_.end()) throw std::out_of_range("original tweet not found");
        const auto id = ids_.next(now);
        tweets_.emplace(id, Tweet{id, authorId, std::move(text), now, {}, repostOf});
        tweetsByAuthor_[authorId].push_back(id);
        return id;
    }

    void like(std::uint64_t userId, std::uint64_t tweetId) {
        std::lock_guard<std::mutex> lock(mutex_);
        static_cast<void>(user(userId));
        auto it = tweets_.find(tweetId);
        if (it == tweets_.end()) throw std::out_of_range("tweet not found");
        it->second.likes.insert(userId);
    }

    std::vector<TimelineEntry> homeTimeline(std::uint64_t userId, std::size_t limit = 20, TimePoint now = Clock::now()) const {
        std::lock_guard<std::mutex> lock(mutex_);
        auto userIt = users_.find(userId);
        if (userIt == users_.end()) throw std::out_of_range("user not found");
        std::unordered_set<std::uint64_t> authors = userIt->second.following;
        authors.insert(userId);
        std::vector<TimelineEntry> result;
        for (const auto authorId : authors) {
            auto authorTweets = tweetsByAuthor_.find(authorId);
            if (authorTweets == tweetsByAuthor_.end()) continue;
            for (const auto tweetId : authorTweets->second) result.push_back({tweetId, rank(tweets_.at(tweetId), now)});
        }
        std::sort(result.begin(), result.end(), [&](const TimelineEntry& left, const TimelineEntry& right) {
            if (left.score != right.score) return left.score > right.score;
            return tweets_.at(left.tweetId).createdAt > tweets_.at(right.tweetId).createdAt;
        });
        if (result.size() > limit) result.resize(limit);
        return result;
    }

    std::vector<Tweet> search(const std::string& query, std::size_t limit = 20) const {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Tweet> result;
        for (const auto& entry : tweets_) if (contains(entry.second.text, query)) result.push_back(entry.second);
        std::sort(result.begin(), result.end(), [](const Tweet& left, const Tweet& right) { return left.createdAt > right.createdAt; });
        if (result.size() > limit) result.resize(limit);
        return result;
    }

    const Tweet& getTweet(std::uint64_t tweetId) const {
        auto it = tweets_.find(tweetId);
        if (it == tweets_.end()) throw std::out_of_range("tweet not found");
        return it->second;
    }
};

} // namespace twitter

int main() {
    using namespace twitter;
    const auto now = Clock::now();
    SocialPlatform platform;
    const auto alice = platform.registerUser("alice");
    const auto bob = platform.registerUser("bob");
    const auto carol = platform.registerUser("carol");
    platform.follow(alice, bob);
    const auto tweet = platform.post(bob, "Distributed systems are fun #systems", now);
    platform.post(carol, "A private timeline post", now);
    platform.like(alice, tweet);
    const auto feed = platform.homeTimeline(alice);
    if (feed.size() != 1 || platform.search("systems").size() != 1) return 1;
    const auto repost = platform.post(alice, "Sharing this", now, tweet);
    if (platform.getTweet(repost).repostOf != tweet) return 1;
    std::cout << "Twitter social checks passed\n";
    std::cout << "Home timeline entries: " << feed.size() << " | Tweet ID: " << tweet << "\n";
}
