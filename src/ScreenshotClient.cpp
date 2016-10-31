#include "ScreenshotClient.hpp"

ScreenshotClient::ScreenshotClient(const char* udid) {
    if (IDEVICE_E_SUCCESS != idevice_new(&device, udid)) {
        if (udid) {
            printf("No device found with udid %s, is it plugged in?\n", udid);
        } else {
            printf("No device found, is it plugged in?\n");
        }
        exit(1);
    }

    if (LOCKDOWN_E_SUCCESS != (ldret = lockdownd_client_new_with_handshake(device, &lckd, NULL))) {
        idevice_free(device);
        printf("ERROR: Could not connect to lockdownd, error code %d\n", ldret);
        exit(1);
    }

    lockdownd_start_service(lckd, "com.apple.mobile.screenshotr", &service);
    lockdownd_client_free(lckd);
    if (service && service->port > 0) {
        if (screenshotr_client_new(device, service, &shotr) != SCREENSHOTR_E_SUCCESS) {
            printf("Could not connect to screenshotr!\n");
            exit(1);
        }
    } else {
        printf("Could not start screenshotr service! Remember that you have to mount the Developer disk image on your device if you want to use the screenshotr service.\n");
        exit(1);
    }
}

ScreenshotClient::~ScreenshotClient() {
    screenshotr_client_free(shotr);
    if (service)
        lockdownd_service_descriptor_free(service);

    idevice_free(device);
}

void ScreenshotClient::capture() {
    if (screenshotr_take_screenshot(shotr, &mData, &mSize) != SCREENSHOTR_E_SUCCESS) {
        printf("Could not get screenshot!\n");
    }
}

char *ScreenshotClient::getData() {
    return mData;
}

uint64_t ScreenshotClient::getSize() {
    return mSize;
}
