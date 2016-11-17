#import "StreamClient.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMediaIO/CMIOHardware.h>
#import <iostream>

#include <TargetConditionals.h>
#include <CoreMedia/CoreMedia.h>

#if TARGET_RT_BIG_ENDIAN
#   define FourCC2Str(fourcc) (const char[]){*((char*)&fourcc), *(((char*)&fourcc)+1), *(((char*)&fourcc)+2), *(((char*)&fourcc)+3),0}
#else
#   define FourCC2Str(fourcc) (const char[]){*(((char*)&fourcc)+3), *(((char*)&fourcc)+2), *(((char*)&fourcc)+1), *(((char*)&fourcc)+0),0}
#endif


@interface VideoSource : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

@property (assign) AVCaptureSession *mSession;
@property (assign) AVCaptureDevice *mDevice;
@property (assign) AVCaptureDeviceInput *mDeviceInput;
@property (assign) AVCaptureVideoDataOutput *mDeviceOutput;

@property (assign) StreamClient *mClient;

- (id) init:(StreamClient *)client;

@end

@implementation VideoSource

- (id) init:(StreamClient *)client ; {
    [super init];

    self.mClient = client;
    self.mSession = [[AVCaptureSession alloc] init];

    return self;
}

- (void)dealloc ; {
    [self.mSession release];
    [self.mDevice release];
    [self.mDeviceOutput release];
    [self.mDeviceInput release];
    [super dealloc];
}

- (bool) setupDevice:(NSString *)udid ; {
    // Waiting for iOS devices to appear after enabling DAL plugins.
    // This is a really ugly place and should be refactored
    for (int i = 0 ; i < 10 and [AVCaptureDevice deviceWithUniqueID: udid] == nil ; i++) {
        [[NSRunLoop mainRunLoop] runUntilDate:[NSDate dateWithTimeIntervalSinceNow:0.1]];
    }

    NSLog(@"Available devices:");
    for (AVCaptureDevice *device in [AVCaptureDevice devicesWithMediaType: AVMediaTypeMuxed]) {
        NSLog(@"%@", device.uniqueID);
    }
    for (AVCaptureDevice *device in [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo]) {
        NSLog(@"%@", device.uniqueID);
    }

    self.mDevice = [AVCaptureDevice deviceWithUniqueID: udid];

    if (self.mDevice == nil) {
        NSLog(@"device with udid '%@' not found", udid);
        return false;
    }
    
    [self.mSession beginConfiguration];

    // Add session input
    NSError *error;
    self.mDeviceInput = [AVCaptureDeviceInput deviceInputWithDevice:self.mDevice error:&error];
    if (self.mDeviceInput == nil) {
        NSLog(@"%@", error);
        return false;
    } else {
        [self.mSession addInput:self.mDeviceInput];
    }
    
    // Add session output
    self.mDeviceOutput = [[AVCaptureVideoDataOutput alloc] init];
    self.mDeviceOutput.alwaysDiscardsLateVideoFrames = YES;
    self.mDeviceOutput.videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:
        AVVideoScalingModeResizeAspect, (id)AVVideoScalingModeKey,
//        [NSNumber numberWithUnsignedInt:400], (id)kCVPixelBufferWidthKey,
//        [NSNumber numberWithUnsignedInt:600], (id)kCVPixelBufferHeightKey,
        [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,
        nil];

    dispatch_queue_t videoQueue = dispatch_queue_create("videoQueue", DISPATCH_QUEUE_SERIAL);

    [self.mDeviceOutput setSampleBufferDelegate:self queue:videoQueue];

    [self.mSession addOutput:self.mDeviceOutput];
    [self.mSession commitConfiguration];
    return true;
}

- (void) captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
    self.mClient->captureOutput(sampleBuffer);
}

@end


struct StreamClientImpl {
    VideoSource* mVideoSource;
};

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


StreamClient::StreamClient() {
    EnableDALDevices();

    impl = new StreamClientImpl();
    impl->mVideoSource = [[VideoSource alloc] init: this];

    mBuffer = 0;
    mLockedBuffer = 0;
}

StreamClient::~StreamClient() {
    if (impl) {
        [impl->mVideoSource release];
    }
    delete impl;
    if (mBuffer) {
        CFRetain(mBuffer);
    }
    if (mLockedBuffer) {
        CFRetain(mLockedBuffer);
    }
}

bool StreamClient::setupDevice(const char *udid) {
    NSString *_udid = [NSString stringWithUTF8String:udid];
    return [impl->mVideoSource setupDevice:_udid];
}

void StreamClient::start() {
    [impl->mVideoSource.mSession startRunning];
}

void StreamClient::stop() {
    [impl->mVideoSource.mSession stopRunning];
}

void StreamClient::captureOutput(CMSampleBufferRef buffer) {
    FrameListener *listener = 0;

    CFRetain(buffer);

    { // scope for the lock
        std::lock_guard<std::mutex> lock(mMutex);
        if (!mBuffer) {
            listener = mFrameListener;
        } else {
            CFRelease(mBuffer);
        }
        mBuffer = buffer;
    }

    if (listener) {
        listener->onFrameAvailable();
    }
}

void StreamClient::setFrameListener(FrameListener *listener) {
    mFrameListener = listener;
}

void StreamClient::lockFrame(Frame *frame) {
    std::lock_guard<std::mutex> lock(mMutex);

    if (!mBuffer) {
        // TODO: handle don't have buffer to lock
        std::cout << "Trying to lockFrame without buffer" << std::endl;
        return;
    }

    if (mLockedBuffer) {
        // TODO: handle already have locked buffer
        std::cout << "Trying to lockFrame, but already have a locked buffer" << std::endl;
        return;
    }

    mLockedBuffer = mBuffer;
    mBuffer = 0;

    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(mLockedBuffer);

    CVPixelBufferLockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    frame->width = CVPixelBufferGetWidth(imageBuffer);
    frame->height = CVPixelBufferGetHeight(imageBuffer);
    frame->data = CVPixelBufferGetBaseAddress(imageBuffer);
    frame->size = CVPixelBufferGetDataSize(imageBuffer);
    frame->bytesPerRow = CVPixelBufferGetBytesPerRow(imageBuffer);
//    OSType format = CVPixelBufferGetPixelFormatType(imageBuffer);
//    NSLog(@"%s", FourCC2Str(format));
//    frame->format = convertFormat(format);
}

void StreamClient::releaseFrame(Frame *frame) {
    std::lock_guard<std::mutex> lock(mMutex);

    if (!mLockedBuffer) {
        // TODO: handle releasing frame without locked buffer
        std::cout << "Trying to releaseFrame without locked buffer" << std::endl;
        return;
    }

    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(mLockedBuffer);
    CVPixelBufferUnlockBaseAddress(imageBuffer, kCVPixelBufferLock_ReadOnly);
    CFRelease(mLockedBuffer);
    mLockedBuffer = 0;
}

void StreamClient::setResolution(uint32_t width, uint32_t height) {
    [impl->mVideoSource.mSession beginConfiguration];
    NSMutableDictionary *settings = [impl->mVideoSource.mDeviceOutput.videoSettings mutableCopy];
    [settings setObject:[NSNumber numberWithUnsignedInt:width] forKey:(id)kCVPixelBufferWidthKey];
    [settings setObject:[NSNumber numberWithUnsignedInt:height] forKey:(id)kCVPixelBufferHeightKey];
    impl->mVideoSource.mDeviceOutput.videoSettings = settings;
    [impl->mVideoSource.mSession commitConfiguration];
}

