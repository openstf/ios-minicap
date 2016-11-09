#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <iostream>

#include "SimpleServer.hpp"
#include "FrameListener.hpp"
#include "Banner.hpp"
#include "JpegEncoder.hpp"
#include "StreamClient.h"
#include "Projection.hpp"

#define OK 0
#define ERROR 1

static FrameListener gWaiter;


void print_usage(int argc, char **argv) {
    char *name = NULL;
    name = strrchr(argv[0], '/');

    printf("Usage: %s [OPTIONS] [FILE]\n", (name ? name + 1: argv[0]));
    printf("Streaming screenshots from a device.\n");
    printf("NOTE: A mounted developer disk image is required on the device, otherwise\n");
    printf("the screenshotr service is not available.\n\n");
    printf("  -u, --udid UDID\ttarget specific device by its 40-digit device UDID\n");
    printf("  -p, --port PORT\tport to run server on\n");
    printf("  -h, --help\t\tprints usage information\n");
    printf("\n");
}


void parse_args(int argc, char **argv, const char **udid, int *port) {
    if (argc == 1 ) {
        print_usage(argc, argv);
        exit(ERROR);
    }
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-u") || !strcmp(argv[i], "--udid")) {
            i++;
            if (!argv[i]) {
                print_usage(argc, argv);
                exit(ERROR);
            }
            *udid = argv[i];
            continue;
        }
        else if (!strcmp(argv[i], "-p") || !strcmp(argv[i], "--port")) {
            i++;
            if (!argv[i]) {
                print_usage(argc, argv);
                exit(ERROR);
            }
            *port = atoi(argv[i]);
            continue;
        }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            print_usage(argc, argv);
            exit(ERROR);

        }
        else {
            print_usage(argc, argv);
            exit(ERROR);
        }
    }
}


static void signal_handler(int signum) {
    switch (signum) {
        case SIGINT:
            printf("Received SIGINT, stopping\n");
            gWaiter.stop();
            break;
        case SIGTERM:
            printf("Received SIGTERM, stopping\n");
            gWaiter.stop();
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
}


int main(int argc, char **argv) {
    const char *udid = NULL;
    int port = 0;

    setup_signal_handler();
    parse_args(argc, argv, &udid, &port);

    StreamClient client;
    if (!client.setupDevice(udid)) {
        return ERROR;
    }
    client.setFrameListener(&gWaiter);
    client.start();

    if (!gWaiter.waitForFrame()) {
        return ERROR;
    }
    client.stop();

    Frame frame;

    client.lockFrame(&frame);
    std::cout << "resolution: " << frame.width << "x" << frame.height << std::endl;
    JpegEncoder encoder(frame.width, frame.height);

    DisplayInfo realInfo, desiredInfo;
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

    char* data = new char[encoder.getBufferSize() + 4];

    while (gWaiter.isRunning() and (socket = server.accept()) > 0) {
        printf("New client connection\n");

        send(socket, banner.getData(), banner.getSize(), 0);

        client.start();
        int pending;
        while (gWaiter.isRunning() and (pending = gWaiter.waitForFrame()) > 0) {
            std::cout << pending << std::endl;
            client.lockFrame(&frame);
            encoder.encode((unsigned char*)frame.data, frame.width, frame.height);

            memcpy(&data[4], encoder.getEncodedData(), encoder.getEncodedSize());
            putUInt32LE(data, encoder.getEncodedSize());
            send(socket, data, encoder.getEncodedSize() + 4, 0);
            client.releaseFrame(&frame);
            std::cout << "send" << std::endl;
        }
    }

    delete [] data;
    return OK;
}
