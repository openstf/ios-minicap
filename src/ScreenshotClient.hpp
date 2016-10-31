#ifndef IOS_MINICAP_SCREENSHOTCLIENT_HPP
#define IOS_MINICAP_SCREENSHOTCLIENT_HPP


#include <cstdint>
#include <stdio.h>
#include <cstdlib>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libimobiledevice/screenshotr.h>


class ScreenshotClient {
public:
    ScreenshotClient(const char* udid);
    ~ScreenshotClient();

    void capture();
    char* getData();
    uint64_t getSize();

private:
    idevice_t device = 0;
    lockdownd_client_t lckd = 0;
    lockdownd_error_t ldret = LOCKDOWN_E_UNKNOWN_ERROR;
    screenshotr_client_t shotr = 0;
    lockdownd_service_descriptor_t service = 0;

    char* mData;
    uint64_t mSize;
};


#endif //IOS_MINICAP_SCREENSHOTCLIENT_HPP
