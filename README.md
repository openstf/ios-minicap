# ios-minicap

iOS Minicap provides a socket interface for streaming realtime screen capture data out of iOS devices. It is built with AVFoundation and iOS Screen mirroring feature.


## Requirements

* brew install libjpeg-turbo (>=1.5 is required)
* Xcode (for the Frameworks)
* [cmake](https://cmake.org/)
* OS X Yosemite (10.9) or higher
* iOS 8 or higher
* [Lightning cable](https://en.wikipedia.org/wiki/Lightning_(connector)). See the list of devices.

## Building

```
./build.sh
```

You should now have the binaries available in `./build`.

## Usage

The minicap protocol is a simple push-based binary protocol. When you first connect to the socket, you get a global header followed by the first frame. The global header will not appear again. More frames keep getting sent until you stop minicap.

Before run, please, check that:

* Node.js 6+ is used (required to run example app.js)
* the computer is trusted by the phone
* the phone screen is not turned off

You can try it using:

```
./run.sh
```

And in another window:


```
cd example
npm install
node app.js
```

Then open http://localhost:9002 in browser

**When device have big FPS, minicap is sending frames to example app trough the localhost connection too quickly, so it could not draw it quick enough. That could cause frames to delay.**

### Global header binary format

Appears once.

| Bytes | Length | Type | Explanation |
|-------|--------|------|-------------|
| 0     | 1 | unsigned char | Version (currently 1) |
| 1     | 1 | unsigned char | Size of the header (from byte 0) |
| 2-5   | 4 | uint32 (low endian) | Pid of the process |
| 6-9   | 4 | uint32 (low endian) | Real display width in pixels |
| 10-13 | 4 | uint32 (low endian) | Real display height in pixels |
| 14-17 | 4 | uint32 (low endian) | Virtual display width in pixels |
| 18-21 | 4 | uint32 (low endian) | Virtual display height in pixels |
| 22    | 1 | unsigned char | Display orientation |
| 23    | 1 | unsigned char | Quirk bitflags (see below) |

#### Quirk bitflags

Currently, the following quirks may be reported:

| Value | Name | Explanation |
|-------|------|-------------|
| 1     | QUIRK_DUMB | Frames will get sent even if there are no changes from the previous frame. Informative, doesn't require any actions on your part. You can limit the capture rate by reading frame data slower in your own code if you wish. |
| 2     | QUIRK_ALWAYS_UPRIGHT | The frame will always be in upright orientation regardless of the device orientation. This needs to be taken into account when rendering the image. |
| 4     | QUIRK_TEAR | Frame tear might be visible. Informative, no action required. Neither of our current two methods exhibit this behavior. |

### Frame binary format

Appears a potentially unlimited number of times.

| Bytes | Length | Type | Explanation |
|-------|--------|------|-------------|
| 0-3   | 4 | uint32 (low endian) | Frame size in bytes (=n) |
| 4-(n+4) | n | unsigned char[] | Frame in JPG format |


## Generation for Xcode

Optionally you may want to use Xcode for developing or building. But still, you should use cmake as a build system.

```
./open_xcode.sh
```
