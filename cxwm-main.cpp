#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iostream>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main(void)
{
    Display *dpy;
    XWindowAttributes attr;
    XButtonEvent start;
    XEvent ev;

    int currentScreen;
    Window root, activeWindow = None; // Variable para la ventana activa

    if (!(dpy = XOpenDisplay(nullptr))) return 1;

    root = DefaultRootWindow(dpy);
    currentScreen = DefaultScreen(dpy);

    std::cout << "CXWM inited! " << DisplayWidth(dpy, currentScreen) << "w "
              << DisplayHeight(dpy, currentScreen) << "h" << std::endl;

    // Configurando la tecla F1, Alt+X y Alt+Shift+Q
    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("F1")), Mod1Mask,
             DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("X")), Mod1Mask,
             DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    XGrabKey(dpy, XKeysymToKeycode(dpy, XStringToKeysym("Q")), Mod1Mask | ShiftMask,
             DefaultRootWindow(dpy), True, GrabModeAsync, GrabModeAsync);

    // Captura los eventos de botón (para mover las ventanas)
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
                }
            }

            if (ev.xkey.keycode == XKeysymToKeycode(dpy, XStringToKeysym("Q")) &&
                (ev.xkey.state & Mod1Mask) && (ev.xkey.state & ShiftMask))
            {
                std::cout << "Quit" << std::endl;
                break;
            }
        }

        else if (ev.type == ButtonPress)
        {
            if (ev.xbutton.subwindow != None)
            {
                activeWindow = ev.xbutton.subwindow; //detecting focus change :3
                std::cout << "actived window changed" << std::endl;
            }

            XGetWindowAttributes(dpy, ev.xbutton.subwindow, &attr);
            start = ev.xbutton;
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
