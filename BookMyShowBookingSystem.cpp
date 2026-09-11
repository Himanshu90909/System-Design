#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace bookmyshow {

using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;
using Seconds = std::chrono::seconds;

enum class SeatState { Available, Held, Booked };
enum class BookingState { Held, Confirmed, Expired, Cancelled };

enum class PaymentState { Authorized, Failed };

struct Seat {
    std::string id;
    std::string row;
    int number;
    SeatState state{SeatState::Available};
};

struct Hold {
    std::string holdId;
    std::string userId;
    std::vector<std::string> seatIds;
    TimePoint expiresAt;
    BookingState state{BookingState::Held};
};

struct Ticket {
    std::string ticketId;
    std::string showId;
    std::string userId;
    std::vector<std::string> seatIds;
    std::string qrPayload;
};

struct BookingRequest {
    std::uint64_t sequence;
    std::string userId;
    std::vector<std::string> seatIds;
};

struct Show {
    std::string id;
    std::string movie;
    std::string theatre;
    std::vector<Seat> seats;
    std::queue<BookingRequest> pending;
    std::unordered_map<std::string, Hold> holds;
    std::unordered_map<std::string, Ticket> tickets;
    std::uint64_t nextSequence{1};
    std::uint64_t nextHold{1};
    std::uint64_t nextTicket{1};
};

class BookingService {
    mutable std::mutex mutex_;
    std::unordered_map<std::string, Show> shows_;
    std::unordered_map<std::string, PaymentState> payments_;

    static std::string id(const std::string& prefix, std::uint64_t value) {
        return prefix + "-" + std::to_string(value);
    }

    Show& show(const std::string& showId) {
        auto it = shows_.find(showId);
        if (it == shows_.end()) throw std::out_of_range("show not found");
        return it->second;
    }

    static Seat& seat(Show& show, const std::string& seatId) {
        auto it = std::find_if(show.seats.begin(), show.seats.end(), [&](const Seat& current) { return current.id == seatId; });
        if (it == show.seats.end()) throw std::out_of_range("seat not found");
        return *it;
    }

    static bool allAvailable(Show& show, const std::vector<std::string>& seatIds) {
        std::unordered_set<std::string> unique;
        for (const auto& seatId : seatIds) {
            if (!unique.insert(seatId).second || seat(show, seatId).state != SeatState::Available) return false;
        }
        return true;
    }

    void expireHolds(Show& show, TimePoint now) {
        for (auto& entry : show.holds) {
            auto& hold = entry.second;
            if (hold.state == BookingState::Held && now >= hold.expiresAt) {
                hold.state = BookingState::Expired;
                for (const auto& seatId : hold.seatIds) seat(show, seatId).state = SeatState::Available;
            }
        }
    }

public:
    void addShow(std::string showId, std::string movie, std::string theatre, std::vector<Seat> seats) {
        std::lock_guard lock(mutex_);
        if (shows_.find(showId) != shows_.end()) throw std::invalid_argument("show already exists");
        const auto key = showId;
        shows_.emplace(key, Show{std::move(showId), std::move(movie), std::move(theatre), std::move(seats)});
    }

    std::vector<Seat> seats(const std::string& showId, TimePoint now = Clock::now()) {
        std::lock_guard lock(mutex_);
        auto& current = show(showId);
        expireHolds(current, now);
        return current.seats;
    }

    std::uint64_t enqueue(const std::string& showId, std::string userId, std::vector<std::string> seatIds) {
        std::lock_guard lock(mutex_);
        auto& current = show(showId);
        if (userId.empty() || seatIds.empty()) throw std::invalid_argument("user and seats are required");
        for (const auto& seatId : seatIds) static_cast<void>(seat(current, seatId));
        const auto sequence = current.nextSequence++;
        current.pending.push({sequence, std::move(userId), std::move(seatIds)});
        return sequence;
    }

    std::optional<std::string> processNext(const std::string& showId, Seconds holdDuration,
                                           TimePoint now = Clock::now()) {
        std::lock_guard lock(mutex_);
        auto& current = show(showId);
        expireHolds(current, now);
        if (current.pending.empty()) return std::nullopt;
        auto request = std::move(current.pending.front());
        current.pending.pop();
        if (!allAvailable(current, request.seatIds)) return std::nullopt;
        const auto holdId = id("hold", current.nextHold++);
        for (const auto& seatId : request.seatIds) seat(current, seatId).state = SeatState::Held;
        current.holds.emplace(holdId, Hold{holdId, std::move(request.userId), std::move(request.seatIds), now + holdDuration});
        return holdId;
    }

    void expire(const std::string& showId, TimePoint now = Clock::now()) {
        std::lock_guard lock(mutex_);
        expireHolds(show(showId), now);
    }

    Ticket confirm(const std::string& showId, const std::string& holdId, const std::string& paymentId,
                   TimePoint now = Clock::now()) {
        std::lock_guard lock(mutex_);
        auto& current = show(showId);
        expireHolds(current, now);
        auto holdIt = current.holds.find(holdId);
        if (holdIt == current.holds.end() || holdIt->second.state != BookingState::Held)
            throw std::runtime_error("hold is missing or expired");
        payments_[paymentId] = PaymentState::Authorized;
        auto& hold = holdIt->second;
        for (const auto& seatId : hold.seatIds) seat(current, seatId).state = SeatState::Booked;
        hold.state = BookingState::Confirmed;
        const auto ticketId = id("ticket", current.nextTicket++);
        Ticket ticket{ticketId, showId, hold.userId, hold.seatIds, "QR:" + ticketId};
        current.tickets.emplace(ticketId, ticket);
        return ticket;
    }

    void cancel(const std::string& showId, const std::string& holdId) {
        std::lock_guard lock(mutex_);
        auto& current = show(showId);
        auto it = current.holds.find(holdId);
        if (it == current.holds.end() || it->second.state != BookingState::Held) throw std::runtime_error("hold cannot be cancelled");
        it->second.state = BookingState::Cancelled;
        for (const auto& seatId : it->second.seatIds) seat(current, seatId).state = SeatState::Available;
    }

    bool validateTicket(const std::string& showId, const std::string& ticketId) const {
        std::lock_guard lock(mutex_);
        auto showIt = shows_.find(showId);
        if (showIt == shows_.end()) return false;
        return showIt->second.tickets.find(ticketId) != showIt->second.tickets.end();
    }
};

} // namespace bookmyshow

int main() {
    using namespace bookmyshow;
    const auto start = Clock::now();
    BookingService service;
    service.addShow("show-1", "System Design: The Movie", "Central Cinema", {
        {"A1", "A", 1}, {"A2", "A", 2}, {"A3", "A", 3}
    });

    const auto first = service.enqueue("show-1", "user-1", {"A1", "A2"});
    const auto second = service.enqueue("show-1", "user-2", {"A1"});
    if (first >= second) return 1;
    const auto hold = service.processNext("show-1", Seconds(600), start);
    if (!hold.has_value()) return 1;
    if (service.processNext("show-1", Seconds(600), start).has_value()) return 1;

    auto ticket = service.confirm("show-1", *hold, "payment-1", start + Seconds(30));
    if (!service.validateTicket("show-1", ticket.ticketId)) return 1;
    if (service.seats("show-1", start + Seconds(30))[0].state != SeatState::Booked) return 1;

    const auto expiredHold = service.enqueue("show-1", "user-3", {"A3"});
    static_cast<void>(expiredHold);
    const auto secondHold = service.processNext("show-1", Seconds(10), start + Seconds(31));
    if (!secondHold.has_value()) return 1;
    service.expire("show-1", start + Seconds(42));
    if (service.seats("show-1", start + Seconds(42))[2].state != SeatState::Available) return 1;

    std::cout << "BookMyShow booking checks passed\n";
    std::cout << "Ticket: " << ticket.ticketId << " | QR: " << ticket.qrPayload << "\n";
}
