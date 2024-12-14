#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>
#include <cstdlib>
#include <inipp.h>
#include <fstream>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main(void)
{

    std::string configAppLauncher,configCloseWindow,configExitWM,configAppLauncherKb;

    const char* userHome = getenv("HOME");
    std::string configPath = std::string(userHome) + "/.config/CXWM";
    std::string autostartPath = configPath + "/Autostart.sh";

    // read settings and stupid things from config.ini
    inipp::Ini<char> ini;
    std::ifstream is(configPath + "/config.ini");
    ini.parse(is);
    
    int configtest = -1;
    inipp::get_value(ini.sections["general"],"appLauncher",configAppLauncher);

    // read keybinds
    inipp::get_value(ini.sections["keybinds"],"closeWindow",configCloseWindow);
    inipp::get_value(ini.sections["keybinds"],"exitWM",configExitWM);
    inipp::get_value(ini.sections["keybinds"],"appLauncherKb",configAppLauncherKb);

    std::cout << configAppLauncher << configCloseWindow << configExitWM << configAppLauncherKb << std::endl;
    
    std::string appLauncher = configAppLauncher;

    // autostart
    int autostartStatus = system(autostartPath.c_str());

    if(autostartStatus != 0)
    {
        std::cout << "Error!,failed to execute autostart script at " << autostartPath << std::endl;
    }

    Display *dpy;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;

    int currentScreen;
    Window root, activeWindow = None;

    if (!(dpy = XOpenDisplay(nullptr))) return 1;

    root = DefaultRootWindow(dpy);
    currentScreen = DefaultScreen(dpy);

    std::cout << "CXWM inited! " << DisplayWidth(dpy, currentScreen) << "w "
              << DisplayHeight(dpy, currentScreen) << "h" << std::endl;

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("F1")), Mod1Mask,
             DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym(configCloseWindow.c_str())), Mod1Mask,
             DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym(configExitWM.c_str())), Mod1Mask | ShiftMask,
             DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym(configAppLauncherKb.c_str())), Mod1Mask,
             DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    
    //mouse buttons,why do most of the popular WMs need a mouse,dude
    XGrabButton(dpy, 1, Mod1Mask, DefaultRootWindow(dpy), True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, DefaultRootWindow(dpy), True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    start.subwindow = None;
    while (true)
    {
        XNextEvent(dpy, &ev);

        if (ev.type == KeyPress)
        {
            if (ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("X")) && (ev.xkey.state & Mod1Mask))
            {
                if (activeWindow != None)
                {
                    std::cout << "killing" << std::endl;
                    XDestroyWindow(dpy, activeWindow);
                    activeWindow = None;
                    start.subwindow = None;
                    ev.xbutton.subwindow = None;
                }
            }

            if (ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("Q")) &&
                (ev.xkey.state & Mod1Mask) && (ev.xkey.state & ShiftMask))
            {
                std::cout << "Quit" << std::endl;
                break;
            }

            if (ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("D")) && (ev.xkey.state & Mod1Mask))
            {
                system(appLauncher.c_str());
            }
        }

        else if (ev.type == ButtonPress)
        {
            if (ev.xbutton.subwindow != None)
            {
                activeWindow = ev.xbutton.subwindow; //detecting focus change :3
                std::cout << "active window changed" << std::endl;
                XRaiseWindow(dpy,activeWindow);

                XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
                start = ev.xbutton;
            }
        }
        else if (ev.type == MotionNotify && start.subwindow != None)
        {
            int xdiff = ev.xbutton.x_root - start.x_root;
            int ydiff = ev.xbutton.y_root - start.y_root;
            XMoveResizeWindow(dpy, start.subwindow,
                              attr.x + (start.button == 1 ? xdiff : 0),
                              attr.y + (start.button == 1 ? ydiff : 0),
                              MAX(1, attr.width + (start.button == 3 ? xdiff : 0)),
                              MAX(1, attr.height + (start.button == 3 ? ydiff : 0)));
        }
        else if (ev.type == ButtonRelease)
            start.subwindow = None;
    }

    XCloseDisplay(dpy);
}
