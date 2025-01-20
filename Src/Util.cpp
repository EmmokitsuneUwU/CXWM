#include <../Include/Util.hpp>
#include <cstring>

namespace CXWMUtil
{
    std::vector<Window> getWindows(Display* dpy)
    {
        Window root = DefaultRootWindow(dpy);
        Window Parent;
        Window *child;
        char* windowName = nullptr;

        unsigned int ammountOfChilds;

        std::vector<Window> Windows;

        if(XQueryTree(dpy,root,&root,&Parent,&child,&ammountOfChilds))
        {
            for(unsigned int i = 0; i < ammountOfChilds; i++)
            {
                if(XFetchName(dpy,child[i],&windowName) && windowName != None && std::strstr(windowName,"Qt Selection Owner")
                     == nullptr && std::strstr(windowName,"CXWMDock") == nullptr && std::strstr(windowName,"MainWindow") == nullptr) //fuck this,i hate it
                {
                    Windows.push_back(child[i]);
                }
            }
            XFree(child);
            std::cout << "OPERATION ENDED" << std::endl;
        }
        else
        {
            std::cout << "ERROR ON CXWMUTIL::GETWINDOWS" << std::endl;
        }
        return Windows;
    }
}