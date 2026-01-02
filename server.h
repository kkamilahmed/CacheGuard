#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include "storage.h"
#include "ratelimiter.h"

void handleClient(SOCKET client_socket, Storage& storage, const std::string& client_ip, RateLimiter& rate_limiter);
void startServer(int portnum);

#endif // SERVER_H