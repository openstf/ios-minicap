#!/usr/bin/env bash

mkdir xcode
cd xcode
cmake -G Xcode ..
open ios_minicap.xcodeproj
