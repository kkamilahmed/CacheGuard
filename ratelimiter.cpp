#include "ratelimiter.h"
#include "utils.h"

void RateLimiter::cleanupOldRequests(const std::string& ip) {
    auto now = std::chrono::steady_clock::now();
    auto& history = request_history_[ip];
    
    while (!history.empty()) {
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - history.front()).count();
        if (age > time_window_) {
            history.pop_front();
        } else {
            break;
        }
    }
}

bool RateLimiter::allowRequest(const std::string& ip) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isBlocked(ip)) {
        return false;
    }
    
    cleanupOldRequests(ip);
    
    auto& history = request_history_[ip];
    
    if (history.size() >= max_requests_) {
        logToFile("ALERT: Rate limit exceeded from " + ip + " (" + std::to_string(history.size()) + " requests in " + std::to_string(time_window_) + "s)");
        
        blocked_ips_[ip] = std::chrono::steady_clock::now();
        logToFile("BLOCKED: IP " + ip + " blocked for 5 minutes due to rate limit violation");
        
        return false;
    }
    
    history.push_back(std::chrono::steady_clock::now());
    
    return true;
}

bool RateLimiter::allowConnection(const std::string& ip) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (isBlocked(ip)) {
        logToFile("BLOCKED: Connection attempt from blocked IP " + ip);
        return false;
    }
    
    int current_connections = active_connections_[ip];
    
    if (current_connections >= max_connections_) {
        logToFile("ALERT: Max connections exceeded from " + ip + " (" + std::to_string(current_connections) + " active connections)");
        
        blocked_ips_[ip] = std::chrono::steady_clock::now();
        logToFile("BLOCKED: IP " + ip + " blocked for 5 minutes due to connection limit violation");
        
        return false;
    }
    
    active_connections_[ip]++;
    
    return true;
}

void RateLimiter::removeConnection(const std::string& ip) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (active_connections_[ip] > 0) {
        active_connections_[ip]--;
    }
    
    if (active_connections_[ip] == 0) {
        active_connections_.erase(ip);
    }
}

bool RateLimiter::isBlocked(const std::string& ip) {
    auto it = blocked_ips_.find(ip);
    if (it != blocked_ips_.end()) {
        auto now = std::chrono::steady_clock::now();
        auto blocked_duration = std::chrono::duration_cast<std::chrono::minutes>(now - it->second).count();
        
        if (blocked_duration >= 5) {
            blocked_ips_.erase(it);
            logToFile("UNBLOCKED: IP " + ip + " unblocked after timeout");
            return false;
        }
        return true;
    }
    return false;
}