#include "SimpleServer.hpp"

#include <fcntl.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstdio>


SimpleServer::SimpleServer(): mFd(0) {

}

SimpleServer::~SimpleServer() {
    if (mFd > 0) {
        ::close(mFd);
    }
}

int SimpleServer::start(int port) {
    int s_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (s_fd < 0) {
        return s_fd;
    }

    int reuse = 1;
    if (setsockopt(s_fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed");
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);

    if (::bind(s_fd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        return -1;
    }

    ::listen(s_fd, 1);

    mFd = s_fd;
    return mFd;
}

int SimpleServer::accept() {
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    return ::accept(mFd, (struct sockaddr *) &addr, &addr_len);
}
