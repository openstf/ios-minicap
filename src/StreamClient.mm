#import "StreamClient.h"
#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMediaIO/CMIOHardware.h>
#import <iostream>


@interface VideoSource : NSObject <AVCaptureVideoDataOutputSampleBufferDelegate>

@property (assign) AVCaptureSession *session;
@property (assign) AVCaptureDevice *device;
@property (assign) AVCaptureDeviceInput *videoDeviceInput;
@property (assign) AVCaptureVideoDataOutput *videoDataOutput;

@property (assign) size_t width;
@property (assign) size_t height;
@property (assign) size_t size;
@property (assign) char* data;
@property (assign) OSType pixelFormat;

@property (assign) StreamClient *client;

- (id) init:(StreamClient *)client;

@end

@implementation VideoSource

- (id) init:(StreamClient *)client; {
    [super init];

    self.client = client;
    self.session = [[AVCaptureSession alloc] init];

    return self;
}

- (void)dealloc
{
    [self.session release];
    [self.device release];
    [self.videoDataOutput release];
    [self.videoDeviceInput release];
    [super dealloc];
}

- (void) waitForDevice ; {
    NSNotificationCenter *notiCenter = [NSNotificationCenter defaultCenter];
    id connObs =[notiCenter addObserverForName:AVCaptureDeviceWasConnectedNotification
            object:nil
    queue:[NSOperationQueue mainQueue]
    usingBlock:^(NSNotification *note)
    {
        NSLog(@"device added");
    }];

    [notiCenter addObserverForName:@"TestNotification"
        object:nil
        queue:[NSOperationQueue mainQueue]
        usingBlock:^(NSNotification *note)
        {
            NSLog(@"device added");
        }];

    [[NSNotificationCenter defaultCenter]
        postNotificationName:@"TestNotification"
        object:self];

    [[NSRunLoop currentRunLoop] run];
}

- (bool) setupDevice:(NSString *)udid ; {

//    [self waitForDevice];
//    NSThread* myThread = [[NSThread alloc] initWithTarget:self
//        selector:@selector(waitForDevice:)
//        object:nil];
//    [myThread start];  // Actually create the thread
    for (AVCaptureDevice *device in [AVCaptureDevice devices]) {
        NSLog(@"%@", device.uniqueID);
    }

    self.device = [AVCaptureDevice deviceWithUniqueID: udid];

    if (self.device == nil) {
        NSLog(@"device with udid '%@' not found", udid);
        return false;
    }
    //    self.device = [AVCaptureDevice devicesWithMediaType:AVMediaTypeMuxed][0];
    //    self.device = [AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo][0];

//        int fps = 5;  // Change this value
//        [self.device lockForConfiguration:nil];
//        [self.device setActiveVideoMinFrameDuration:CMTimeMake(1, fps)];
//        [self.device setActiveVideoMaxFrameDuration:CMTimeMake(1, fps)];
//        [self.device unlockForConfiguration];

    [self.session beginConfiguration];

    //    [self configureCaptureSettings];

    // Add session input
    NSError *error;
    self.videoDeviceInput = [AVCaptureDeviceInput deviceInputWithDevice:self.device error:&error];
    if (self.videoDeviceInput == nil) {
        dispatch_async(dispatch_get_main_queue(), ^(void) {
            NSLog(@"%@", error);
        });
        return false;
    } else {
        [self.session addInput:self.videoDeviceInput];
    }

    // Add session output
    self.videoDataOutput = [[AVCaptureVideoDataOutput alloc] init];
    self.videoDataOutput.alwaysDiscardsLateVideoFrames = YES;
    self.videoDataOutput.videoSettings = [NSDictionary dictionaryWithObjectsAndKeys:
//            AVVideoScalingModeResizeAspectFill, (id)AVVideoScalingModeKey,
//            [NSNumber numberWithDouble:320.0], (id)kCVPixelBufferWidthKey,
    //        [NSNumber numberWithDouble:600.0], (id)kCVPixelBufferHeightKey,
        [NSNumber numberWithUnsignedInt:kCVPixelFormatType_32BGRA], (id)kCVPixelBufferPixelFormatTypeKey,
        nil];

    dispatch_queue_t videoQueue = dispatch_queue_create("videoQueue", DISPATCH_QUEUE_SERIAL);

    [self.videoDataOutput setSampleBufferDelegate:self queue:videoQueue];

    [self.session addOutput:self.videoDataOutput];
    [self.session commitConfiguration];
    return true;
}

- (void) captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
    self.client->captureOutput(sampleBuffer);
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
    mData = 0;
    mSize = 0;
    mWidth = 0;
    mHeight = 0;
    mFrameAvailable = false;

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
    [impl->mVideoSource.session startRunning];
}

void StreamClient::stop() {
    [impl->mVideoSource.session stopRunning];
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
        // TODO: handle don't have any buffer
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
    //    frame->format = CVPixelBufferGetPixelFormatType(imageBuffer);
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
