#pragma once
#include <vector>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <iostream>

namespace CXWMUtil
{
    std::vector<Window> getWindows(Display* dpy);
}