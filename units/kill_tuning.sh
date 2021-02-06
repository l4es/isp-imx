#!/bin/sh
ps -ef | grep tuning-server | grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | grep tuning-server.s | grep -v grep | awk '{print $2}' | xargs kill -9
ps -ef | grep tuning-client | grep -v grep | awk '{print $2}' | xargs kill -9
