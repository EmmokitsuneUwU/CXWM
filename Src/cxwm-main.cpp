#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include <iostream>
#include <cstdlib>
#include <inipp.h>
#include <fstream>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

//stupid line for config variables
std::string configAppLauncher,configAppsPath,configTerminal,
    configCloseWindow,configExitWM,configAppLauncherKb,configTerminalKb;

int backgroundR = 0,backgroundG = 0,backgroundB = 0;

const char* userHome = getenv("HOME");
std::string configPath = std::string(userHome) + "/.config/CXWM";
std::string autostartPath = configPath + "/Autostart.sh";
    
std::string appLauncher = configAppLauncher;

void loadSettings()
{
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

    // read colors
    inipp::get_value(ini.sections["colors"],"backgroundR",backgroundR);
    inipp::get_value(ini.sections["colors"],"backgroundG",backgroundG);
    inipp::get_value(ini.sections["colors"],"backgroundB",backgroundB);
}
void grabKeys(Display *dpy)
{
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
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("m") ), Mod1Mask,
        DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);
    
    //mouse buttons,why do most of the popular WMs need a mouse,dude
    XGrabButton(dpy, 1, Mod1Mask, DefaultRootWindow(dpy), True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    XGrabButton(dpy, 3, Mod1Mask, DefaultRootWindow(dpy), True,
                ButtonPressMask | ButtonReleaseMask | PointerMotionMask, GrabModeAsync, GrabModeAsync, None, None);
}

int main(void)
{
    loadSettings();

    Display *dpy;
    XWindowAttributes attr;
    XClassHint classHint;
    XButtonEvent start;
    XEvent ev;

    XColor colorBackground;
    colorBackground.red = backgroundR * 257;
    colorBackground.green = backgroundG * 257;
    colorBackground.blue = backgroundB * 257;
    colorBackground.flags = DoRed | DoGreen | DoBlue;

    int currentScreen;
    int revert_to;
    Window root, activeWindow = None,focusedWindow = None,backgroundWindow = None;
    static Window previousWindow = None;

    if (!(dpy = XOpenDisplay(nullptr))) return 1;

    root = DefaultRootWindow(dpy);
    currentScreen = DefaultScreen(dpy);

    Colormap colormap = DefaultColormap(dpy,currentScreen);

    if(!XAllocColor(dpy,colormap,&colorBackground))
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

    grabKeys(dpy);
    
    // Low level autostarts
    system((configAppsPath + "CXWMWelcome/CXWMWelcome &").c_str());
    system((configAppsPath + "CXWMDock/CXWMDock &").c_str());
    // autostart
    int autostartStatus = system(autostartPath.c_str());

    if(autostartStatus != 0)
    {
        std::cout << "Error!,failed to execute autostart script at " << autostartPath << std::endl;
    }

    backgroundWindow = XCreateSimpleWindow(dpy,root,0,0,DisplayWidth(dpy,currentScreen),DisplayHeight(dpy,currentScreen),1,BlackPixel(dpy,currentScreen),colorBackground.pixel);

    XMapWindow(dpy,backgroundWindow);

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
                if (activeWindow != None && XGetClassHint(dpy,activeWindow,&classHint))
                {
                    if(strcmp(classHint.res_name,"CXWMDock") != 0)
                    {
                        XEvent event;
                        event.xclient.type = ClientMessage;
                        event.xclient.window = activeWindow;
                        event.xclient.message_type = XInternAtom(dpy, "WM_PROTOCOLS", true);
                        event.xclient.format = 32;
                        event.xclient.data.l[0] = XInternAtom(dpy, "WM_DELETE_WINDOW", false);
                        event.xclient.data.l[1] = CurrentTime;
                        XSendEvent(dpy, activeWindow, False, NoEventMask, &event);

                        activeWindow = None;
                        ev.xbutton.subwindow = None;
                        previousWindow = None;
                        XSetInputFocus(dpy,root,RevertToParent,CurrentTime);
                    }

                }
            }
            if(ev.xkey.keycode == XKeysymToKeycode(dpy,XStringToKeysym("m")) && (ev.xkey.state & Mod1Mask))
            {
                if(activeWindow != None && XGetClassHint(dpy,activeWindow,&classHint))
                {
                    if(strcmp(classHint.res_name,"CXWMDock") != 0 && strcmp(classHint.res_name,"CXWMWelcome") != 0)
                    {
                        XRaiseWindow(dpy,activeWindow);
                        XMoveResizeWindow(dpy,activeWindow,0,0,DisplayWidth(dpy,currentScreen),DisplayHeight(dpy,currentScreen));
                    }
                    if(classHint.res_name) { XFree(classHint.res_name); }
                    if(classHint.res_class) { XFree(classHint.res_class); }
                }
            }

            if (ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym(configExitWM.c_str())) &&
                (ev.xkey.state & Mod1Mask) && (ev.xkey.state & ShiftMask))
            {
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
            if (ev.xbutton.subwindow != None && ev.xbutton.subwindow != backgroundWindow)
            {
                activeWindow = ev.xbutton.subwindow; //detecting focus change :3

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
                    if(strcmp(classHint.res_name,"CXWMDock") !=0)
                    {
                        int xdiff = ev.xbutton.x_root - start.x_root;
                        int ydiff = ev.xbutton.y_root - start.y_root;
                        XMoveWindow(dpy,activeWindow,attr.x + (start.button == 1 ? xdiff : 0),attr.y + (start.button == 1 ? ydiff : 0));
                    }
                    if(classHint.res_name) { XFree(classHint.res_name); }
                    if(classHint.res_class) { XFree(classHint.res_class); }
                }
            }
            //Change size
            else if (activeWindow != None && start.button ==3)
            {
                if(XGetClassHint(dpy,activeWindow,&classHint))
                {
                    if(strcmp(classHint.res_name,"CXWMWelcome") !=0 && strcmp(classHint.res_name,"CXWMDock") != 0)
                    {
                        int xdiff = ev.xbutton.x_root - start.x_root;
                        int ydiff = ev.xbutton.y_root - start.y_root;
                        XResizeWindow(dpy, activeWindow,
                            MAX(1, attr.width + (start.button == 3 ? xdiff : 0)),
                            MAX(1, attr.height + (start.button == 3 ? ydiff : 0)));
                    }
                    if(classHint.res_name) { XFree(classHint.res_name); }
                    if(classHint.res_class) { XFree(classHint.res_class); }
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
