#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <zconf.h>

#include <iostream>
#include <thread>

#include "SimpleServer.hpp"
#include "FrameListener.hpp"
#include "Banner.hpp"
#include "JpegEncoder.hpp"
#include "StreamClient.h"
#import <CoreMediaIO/CMIOHardware.h>
#import <CoreFoundation/CoreFoundation.h>

// MSG_NOSIGNAL does not exists on OS X
#if defined(__APPLE__) || defined(__MACH__)
# ifndef MSG_NOSIGNAL
#   define MSG_NOSIGNAL SO_NOSIGPIPE
# endif
#endif

//static FrameListener gWaiter;


void print_usage(char **argv) {
    char *name = NULL;
    name = strrchr(argv[0], '/');

    printf("Usage: %s [OPTIONS]\n", (name ? name + 1: argv[0]));
    printf("Stream video from a device.\n");
    printf("  -u1, --udid1 UDID\t\ttarget specific device by its 40-digit device UDID\n");
    printf("  -u2, --udid2 UDID\t\ttarget specific device by its 40-digit device UDID\n");
    printf("  -p, --port PORT\t\tport to run server on\n");
    printf("  -r, --resolution RESOLUTION\tdesired resolution <w>x<h>\n");
    printf("  -h, --help\t\t\tprints usage information\n");
    printf("\n");
}


bool parse_args(int argc, char **argv, const char **udid1, const char **udid2, int *port, const char **resolution) {
    if ( argc < 7 ) {
        // Currently the easiest way to make all arguments required
        print_usage(argv);
        return false;
    }
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-u1") || !strcmp(argv[i], "--udid1")) {
            i++;
            if (!argv[i]) {
                print_usage(argv);
                return false;
            }
            *udid1 = argv[i];
            continue;
        }
        if (!strcmp(argv[i], "-u2") || !strcmp(argv[i], "--udid2")) {
            i++;
            if (!argv[i]) {
                print_usage(argv);
                return false;
            }
            *udid2 = argv[i];
            continue;
        }
        else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")) {
            i++;
            if (!argv[i]) {
                print_usage(argv);
                return false;
            }
            *port = atoi(argv[i]);
            continue;
        }
        else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--resolution")) {
            i++;
            if (!argv[i]) {
                print_usage(argv);
                return false;
            }
            *resolution = argv[i];
            continue;
        }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_usage(argv);
            return false;

        }
        else {
            print_usage(argv);
            return false;
        }
    }
    return true;
}


static void signal_handler(int signum) {
    switch (signum) {
        case SIGINT:
            printf("Received SIGINT, stopping\n");
            CFRunLoopStop(CFRunLoopGetMain());
            break;
        case SIGTERM:
            printf("Received SIGTERM, stopping\n");
            CFRunLoopStop(CFRunLoopGetMain());
            break;
        default:
            abort();
    }
}


static void setup_signal_handler() {
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    // we want to just ignore the SIGPIPE and get a EPIPE when socket is closed
    signal(SIGPIPE, SIG_IGN);
}


static ssize_t pumps(int fd, unsigned char* data, size_t length) {
    do {
        // SIGPIPE is set to ignored so we will just get EPIPE instead
        ssize_t wrote = send(fd, data, length, 0);

        if (wrote < 0) {
            return wrote;
        }

        data += wrote;
        length -= wrote;
    }
    while (length > 0);

    return 0;
}


void parseResolution(const char* resolution, uint32_t* width, uint32_t* height) {
    std::string _resolution(resolution);
    size_t sep = _resolution.find("x");
    *width = std::stoul(_resolution.substr(0, sep).c_str());
    *height = std::stoul(_resolution.substr(sep+1, _resolution.length()).c_str());
}


int streamDevice(int port, const char* udid, int width, int height) {
    FrameListener waiter;
    StreamClient client;
    if (!client.setupDevice(udid)) {
        return EXIT_FAILURE;
    }
    client.setResolution(width, height);
    client.setFrameListener(&waiter);
    client.start();

    if (!waiter.waitForFrame()) {
        return EXIT_SUCCESS;
    }
    client.stop();

    Frame frame;

    client.lockFrame(&frame);
    std::cout << "resolution: " << frame.width << "x" << frame.height << std::endl;
    JpegEncoder encoder(&frame);

    DeviceInfo realInfo, desiredInfo;
    realInfo.orientation = 0;
    realInfo.height = frame.height;
    realInfo.width = frame.width;
    desiredInfo.orientation = 0;
    desiredInfo.height = frame.height;
    desiredInfo.width = frame.width;

    Banner banner(realInfo, desiredInfo);
    client.releaseFrame(&frame);


    SimpleServer server;
    server.start(port);
    int socket;

    unsigned char frameSize[4];
    while (waiter.isRunning() and (socket = server.accept()) > 0) {
        std::cout << "New client connection" << std::endl;

        send(socket, banner.getData(), banner.getSize(), 0);

        client.start();
        while (waiter.isRunning() and waiter.waitForFrame() > 0) {
            client.lockFrame(&frame);
            encoder.encode(&frame);
            client.releaseFrame(&frame);
            putUInt32LE(frameSize, encoder.getEncodedSize());
            if ( pumps(socket, frameSize, 4) < 0 ) {
                break;
            }
            if ( pumps(socket, encoder.getEncodedData(), encoder.getEncodedSize()) < 0 ) {
                break;
            }
        }
        client.stop();
    }
}


void EnableDALDevices()
{
    std::cout << "EnableDALDevices" << std::endl;
    CMIOObjectPropertyAddress prop = {
            kCMIOHardwarePropertyAllowScreenCaptureDevices,
            kCMIOObjectPropertyScopeGlobal,
            kCMIOObjectPropertyElementMaster
    };
    UInt32 allow = 1;
    CMIOObjectSetPropertyData(kCMIOObjectSystemObject,
                              &prop, 0, NULL,
                              sizeof(allow), &allow );
}


int main(int argc, char **argv) {
    const char *udid1 = NULL;
    const char *udid2 = NULL;
    const char *resolution = NULL;
    int port = 0;

    setup_signal_handler();
    if ( !parse_args(argc, argv, &udid1, &udid2, &port, &resolution) ) {
        return EXIT_FAILURE;
    }

    uint32_t width = 0, height = 0;
    parseResolution(resolution, &width, &height);

    for (auto const& _udid: StreamClient::getUdids()) {
        std::cout << _udid << std::endl;
    }

    EnableDALDevices();
    std::thread thr(streamDevice, port, udid1, width, height);
    std::thread thr2(streamDevice, port+1, udid2, width, height);
    CFRunLoopRun();
    return EXIT_SUCCESS;
}
