To build for ANDROID, please follow below steps:

1. Install android sdk r21. If the sdk is already installed, go the next step.
   1) cd /opt
   2) sudo wget https://dl.google.com/android/repository/android-ndk-r21-linux-x86_64.zip
   3) sudo unzip android-ndk-r21-linux-x86_64.zip

2. Generate Makefile
   1) cd appshell
   2) mkdir build-android
   3) cp android-build.sh build-android
   4) cd build-android
   5) ./android-build.sh

3. Build
   1) make

