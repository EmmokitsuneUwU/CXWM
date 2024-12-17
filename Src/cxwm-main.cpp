#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <iostream>
#include <cstdlib>
#include <inipp.h>
#include <fstream>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main(void)
{
    //stupid line for config variables
    std::string configAppLauncher,configAppsPath,configTerminal,
        configCloseWindow,configExitWM,configAppLauncherKb,configTerminalKb;

    const char* userHome = getenv("HOME");
    std::string configPath = std::string(userHome) + "/.config/CXWM";
    std::string autostartPath = configPath + "/Autostart.sh";

    // read settings and stupid things from config.ini
    inipp::Ini<char> ini;
    std::ifstream is(configPath + "/config.ini");
    ini.parse(is);
    
    inipp::get_value(ini.sections["general"],"appLauncher",configAppLauncher);
    inipp::get_value(ini.sections["general"],"appsPath",configAppsPath);
    inipp::get_value(ini.sections["general"],"terminal",configTerminal);

    // read keybinds
    inipp::get_value(ini.sections["keybinds"],"closeWindow",configCloseWindow);
    inipp::get_value(ini.sections["keybinds"],"exitWM",configExitWM);
    inipp::get_value(ini.sections["keybinds"],"appLauncherKb",configAppLauncherKb);
    inipp::get_value(ini.sections["keybinds"],"terminalKb",configTerminalKb);
    
    std::string appLauncher = configAppLauncher;

    Display *dpy;
    XWindowAttributes attr;
    XClassHint classHint;
    XButtonEvent start;
    XEvent ev;

    XColor colorBorder;
    colorBorder.red = 143 * 257;
    colorBorder.green = 142 * 257;
    colorBorder.blue = 154 * 257;
    colorBorder.flags = DoRed | DoGreen | DoBlue;

    int currentScreen;
    int revert_to;
    Window root, activeWindow = None,focusedWindow = None;
    static Window previousWindow = None;

    if (!(dpy = XOpenDisplay(nullptr))) return 1;

    root = DefaultRootWindow(dpy);
    currentScreen = DefaultScreen(dpy);

    Colormap colormap = DefaultColormap(dpy,currentScreen);

    if(!XAllocColor(dpy,colormap,&colorBorder))
    {
        std::cout << "Error,cant alloc colorBorder" << std::endl;
    }

    Atom net_wm_name = XInternAtom(dpy, "_NET_WM_NAME", False);
    Atom utf8_string = XInternAtom(dpy, "UTF8_STRING", False);

    const char* wm_name = "CXWM";

    XChangeProperty(dpy, root, net_wm_name, utf8_string, 8, PropModeReplace,
        reinterpret_cast<const unsigned char*>(wm_name), strlen(wm_name));

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

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("Return") ), Mod1Mask,
        DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("R") ), Mod1Mask,
        DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    
    //mouse buttons,why do most of the popular WMs need a mouse,dude
    XGrabButton(dpy, 1, Mod1Mask, DefaultRootWindow(dpy), True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, DefaultRootWindow(dpy), True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);

    // Low level autostarts
    system((configAppsPath + "CXWMWelcome/CXWMWelcome &").c_str());
    // autostart
    int autostartStatus = system(autostartPath.c_str());

    if(autostartStatus != 0)
    {
        std::cout << "Error!,failed to execute autostart script at " << autostartPath << std::endl;
    }

    while (true)
    {
        XNextEvent(dpy, &ev);

        if(XGetInputFocus(dpy,&focusedWindow,&revert_to) == None)
        {
            XSetInputFocus(dpy,root,RevertToParent,CurrentTime);
        }

        if (ev.type == KeyPress)
        {
            if (ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(configCloseWindow.c_str())) && (ev.xkey.state & Mod1Mask))
            {
                if (activeWindow != None)
                {
                    std::cout << "killing" << std::endl;
                    XDestroyWindow(dpy, activeWindow);
                    activeWindow = None;
                    ev.xbutton.subwindow = None;
                    previousWindow = None;
                    XSetInputFocus(dpy,root,RevertToParent,CurrentTime);
                }
            }

            if (ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(configExitWM.c_str())) &&
                (ev.xkey.state & Mod1Mask) && (ev.xkey.state & ShiftMask))
            {
                std::cout << "Quit" << std::endl;
                break;
            }

            if (ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(configAppLauncherKb.c_str())) && (ev.xkey.state & Mod1Mask))
            {
                system(appLauncher.c_str());
            }
            if (ev.xkey.keycode == XKeysymToKeycode(dpy,XStringToKeysym("Return") ) && (ev.xkey.state & Mod1Mask) )
            {
                system((configTerminal + "&").c_str());
            }
            if (ev.xkey.keycode == XKeysymToKeycode(dpy,XStringToKeysym("R") ) && (ev.xkey.state & Mod1Mask) )
            {
                system((configAppsPath + "CXWMAppRunner/CXWMAppRunner &").c_str());
            }
        }

        else if (ev.type == ButtonPress)
        {
            if (ev.xbutton.subwindow != None)
            {
                activeWindow = ev.xbutton.subwindow; //detecting focus change :3
                std::cout << "active window changed" << std::endl;
                XRaiseWindow(dpy,activeWindow);
                XSetInputFocus(dpy,activeWindow,RevertToParent,CurrentTime);
                previousWindow = activeWindow;

                XGetWindowAttributes(dpy, activeWindow, &attr);
                start = ev.xbutton;
            }
        }
        else if (ev.type == MotionNotify)
        {
            //Move
            if(activeWindow != None && start.button == 1)
            {
                if(XGetClassHint(dpy,activeWindow,&classHint))
                {
                    if(strcmp(classHint.res_name,"") !=0)
                    {
                        int xdiff = ev.xbutton.x_root - start.x_root;
                        int ydiff = ev.xbutton.y_root - start.y_root;
                        XMoveWindow(dpy,activeWindow,attr.x + (start.button == 1 ? xdiff : 0),attr.y + (start.button == 1 ? ydiff : 0));
                    }
                }
            }
            //Change size
            else if (activeWindow != None && start.button ==3)
            {
                if(XGetClassHint(dpy,activeWindow,&classHint))
                {
                    if(strcmp(classHint.res_name,"CXWMWelcome") !=0)
                    {
                        int xdiff = ev.xbutton.x_root - start.x_root;
                        int ydiff = ev.xbutton.y_root - start.y_root;
                        XResizeWindow(dpy, activeWindow,
                            MAX(1, attr.width + (start.button == 3 ? xdiff : 0)),
                            MAX(1, attr.height + (start.button == 3 ? ydiff : 0)));
                    }
                }
            }
        }
        else if (ev.type == ButtonRelease)
        {
            activeWindow = None;
        }

    }

    XCloseDisplay(dpy);
}
