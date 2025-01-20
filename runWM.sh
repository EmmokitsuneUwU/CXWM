#!/bin/bash

pkill Xephyr 
Xephyr :2 -screen 1280x720 &

sleep 0.2

DISPLAY=:2 ./Build/CXWM