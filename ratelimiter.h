#ifndef RATELIMITER_H
#define RATELIMITER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <deque>

class RateLimiter {
public:
    RateLimiter(int max_requests = 100, int time_window_seconds = 60, int max_connections = 10)
        : max_requests_(max_requests), 
          time_window_(time_window_seconds),
          max_connections_(max_connections) {}

    bool allowRequest(const std::string& ip);
    bool allowConnection(const std::string& ip);
    void removeConnection(const std::string& ip);
    bool isBlocked(const std::string& ip);

private:
    void cleanupOldRequests(const std::string& ip);
    
    int max_requests_;
    int time_window_;
    int max_connections_;
    
    std::mutex mutex_;
    std::unordered_map<std::string, std::deque<std::chrono::steady_clock::time_point>> request_history_;
    std::unordered_map<std::string, int> active_connections_;
    std::unordered_map<std::string, std::chrono::steady_clock::time_point> blocked_ips_;
};

#endif // RATE_LIMITER_H