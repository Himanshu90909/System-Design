#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

namespace urlshortener {

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;

struct UrlRecord {
    std::string shortCode;
    std::string longUrl;
    TimePoint createdAt;
    std::optional<TimePoint> expiresAt;
    bool active{true};
    std::uint64_t clickCount{0};
};

struct CreateRequest {
    std::string longUrl;
    std::optional<std::string> customAlias;
    std::optional<TimePoint> expiresAt;
};

struct ClickEvent {
    std::string shortCode;
    TimePoint accessedAt;
};

class ShortCodeGenerator {
public:
    virtual ~ShortCodeGenerator() = default;
    virtual std::string next() = 0;
};

class Base62CodeGenerator final : public ShortCodeGenerator {
    static constexpr char ALPHABET[] =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::atomic<std::uint64_t> sequence_;

public:
    explicit Base62CodeGenerator(std::uint64_t initial = 1000000) : sequence_(initial) {}

    std::string next() override {
        auto value = sequence_.fetch_add(1, std::memory_order_relaxed);
        std::string code;
        do {
            code.push_back(ALPHABET[value % 62]);
            value /= 62;
        } while (value > 0);
        std::reverse(code.begin(), code.end());
        return code;
    }
};

class UrlShortener {
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, UrlRecord> records_;
    std::vector<ClickEvent> events_;
    std::unique_ptr<ShortCodeGenerator> generator_;

    static bool isValidUrl(const std::string& url) {
        const auto scheme = url.find("://");
        if (scheme == std::string::npos || scheme == 0 || scheme + 3 >= url.size()) return false;
        const auto hostStart = scheme + 3;
        return url.find_first_of(" \t\r\n", hostStart) == std::string::npos &&
               url.find('.', hostStart) != std::string::npos;
    }

    static bool isValidAlias(const std::string& alias) {
        if (alias.empty() || alias.size() > 64) return false;
        return std::all_of(alias.begin(), alias.end(), [](unsigned char c) {
            return std::isalnum(c) || c == '-' || c == '_';
        });
    }

    static bool isExpired(const UrlRecord& record, TimePoint now) {
        return record.expiresAt.has_value() && now >= *record.expiresAt;
    }

public:
    explicit UrlShortener(std::unique_ptr<ShortCodeGenerator> generator =
                              std::make_unique<Base62CodeGenerator>())
        : generator_(std::move(generator)) {}

    UrlRecord create(const CreateRequest& request, TimePoint now = Clock::now()) {
        if (!isValidUrl(request.longUrl)) throw std::invalid_argument("longUrl must be a valid URL");
        if (request.expiresAt && *request.expiresAt <= now)
            throw std::invalid_argument("expiresAt must be in the future");

        std::unique_lock lock(mutex_);
        std::string code;
        if (request.customAlias) {
            if (!isValidAlias(*request.customAlias)) throw std::invalid_argument("invalid customAlias");
            code = *request.customAlias;
            if (records_.find(code) != records_.end()) throw std::invalid_argument("customAlias is already taken");
        } else {
            do { code = generator_->next(); } while (records_.find(code) != records_.end());
        }

        UrlRecord record{code, request.longUrl, now, request.expiresAt, true, 0};
        records_.emplace(code, record);
        return record;
    }

    std::string resolve(const std::string& code, TimePoint now = Clock::now()) {
        std::unique_lock lock(mutex_);
        auto it = records_.find(code);
        if (it == records_.end()) throw std::out_of_range("short code not found");
        if (!it->second.active || isExpired(it->second, now)) throw std::runtime_error("short URL is inactive or expired");
        ++it->second.clickCount;
        events_.push_back({code, now});
        return it->second.longUrl;
    }

    UrlRecord get(const std::string& code, TimePoint now = Clock::now()) const {
        std::shared_lock lock(mutex_);
        auto it = records_.find(code);
        if (it == records_.end()) throw std::out_of_range("short code not found");
        UrlRecord result = it->second;
        if (isExpired(result, now)) result.active = false;
        return result;
    }

    void disable(const std::string& code) {
        std::unique_lock lock(mutex_);
        auto it = records_.find(code);
        if (it == records_.end()) throw std::out_of_range("short code not found");
        it->second.active = false;
    }

    void updateExpiry(const std::string& code, std::optional<TimePoint> expiresAt,
                      TimePoint now = Clock::now()) {
        if (expiresAt && *expiresAt <= now) throw std::invalid_argument("expiresAt must be in the future");
        std::unique_lock lock(mutex_);
        auto it = records_.find(code);
        if (it == records_.end()) throw std::out_of_range("short code not found");
        it->second.expiresAt = expiresAt;
    }

    std::vector<ClickEvent> history(const std::string& code) const {
        std::shared_lock lock(mutex_);
        std::vector<ClickEvent> result;
        for (const auto& event : events_) if (event.shortCode == code) result.push_back(event);
        return result;
    }
};

} // namespace urlshortener

int main() {
    using namespace urlshortener;
    const auto now = Clock::now();
    UrlShortener service;

    auto generated = service.create({"https://example.com/articles/system-design", std::nullopt, std::nullopt}, now);
    auto custom = service.create({"https://example.com/architecture", std::string("sysdesign"), now + std::chrono::hours(1)}, now);

    if (service.resolve(generated.shortCode, now) != "https://example.com/articles/system-design") return 1;
    if (service.resolve("sysdesign", now) != "https://example.com/architecture") return 1;
    if (service.get(generated.shortCode, now).clickCount != 1) return 1;
    if (service.history("sysdesign").size() != 1) return 1;

    service.disable(generated.shortCode);
    try { service.resolve(generated.shortCode, now); return 1; }
    catch (const std::runtime_error&) {}

    try { service.create({"not-a-url", std::nullopt, std::nullopt}, now); return 1; }
    catch (const std::invalid_argument&) {}

    std::cout << "All URL shortener checks passed\n";
    std::cout << "Generated code: " << generated.shortCode << "\n";
    std::cout << "Custom code: " << custom.shortCode << "\n";
}
