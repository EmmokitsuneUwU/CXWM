#!/bin/bash

pkill Xephyr 
Xephyr :1 -screen 1280x720 &

sleep 0.2

DISPLAY=:1 ./Build/CXWM