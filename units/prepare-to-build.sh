#!/bin/sh

BUILD=BUILD

mkdir -p $BUILD

BIN=generated/Debug/bin

mkdir -p $BIN

cd $BIN

ln -sf ../../../test/case

cd -