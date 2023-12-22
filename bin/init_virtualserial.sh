#!/bin/sh
socat -d -d pty,b115200,raw,echo=0 pty,b115200,raw,echo=0 &