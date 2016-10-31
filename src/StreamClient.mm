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

- (void) waitForDevice ; {
    NSNotificationCenter *notiCenter = [NSNotificationCenter defaultCenter];
    id connObs =[notiCenter addObserverForName:AVCaptureDeviceWasConnectedNotification
            object:nil
    queue:[NSOperationQueue mainQueue]
    usingBlock:^(NSNotification *note)
    {
        NSLog(@"device added");
    }];
    [[NSRunLoop mainRunLoop] run];
}

- (bool) setupDevice:(NSString *)udid; {

    [self waitForDevice];
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

        int fps = 5;  // Change this value
        [self.device lockForConfiguration:nil];
        [self.device setActiveVideoMinFrameDuration:CMTimeMake(1, fps)];
    //    [self.device setActiveVideoMaxFrameDuration:CMTimeMake(1, fps)];
        [self.device unlockForConfiguration];

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

    dispatch_queue_t videoQueue = dispatch_queue_create("videoQueue", NULL);

    [self.videoDataOutput setSampleBufferDelegate:self queue:videoQueue];
    [self.session addOutput:self.videoDataOutput];

    [self.session commitConfiguration];
    return true;
}

- (void) captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection {
    CVImageBufferRef imageBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);

    CVPixelBufferLockBaseAddress(imageBuffer, 0);

    self.width = CVPixelBufferGetWidth(imageBuffer);
    self.height = CVPixelBufferGetHeight(imageBuffer);
    self.size = CVPixelBufferGetDataSize(imageBuffer);
    self.pixelFormat = CVPixelBufferGetPixelFormatType(imageBuffer);
    self.data = (char*)CVPixelBufferGetBaseAddress(imageBuffer);

    self.client->newFrame(self.data, self.size, self.width, self.height);

    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);
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
}

StreamClient::~StreamClient() {
    if (impl) {
        [impl->mVideoSource release];
    }
    delete impl;
    if(mFrameAvailable) {
        delete mData;
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

void StreamClient::newFrame(char* data, size_t size, uint32_t width, uint32_t height) {
    if(!mFrameAvailable){
        std::cout << "initializing frame buffer" << std::endl;
        mData = new char[size];
    }
    if (memcmp(mData, data, size) != 0) {
        memcpy(mData, data, size);
        mFrameAvailable = true;
        if (mFrameListener) {
            mFrameListener->onFrameAvailable();
        }
    }
    mSize = size;
    mWidth = width;
    mHeight = height;
}

void StreamClient::setFrameListener(FrameListener *listener) {
    mFrameListener = listener;
}

void StreamClient::getFrame(Frame *frame) {
    frame->data = mData;
    frame->size = mSize;
    frame->width = mWidth;
    frame->height = mHeight;
}
