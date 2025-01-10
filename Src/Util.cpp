#include <../Include/Util.hpp>

namespace CXWMUtil
{
    std::vector<Window> getWindows(Display* dpy)
    {
        Window root = DefaultRootWindow(dpy);
        Window Parent;
        Window *child;
        unsigned int ammountOfChilds;

        std::vector<Window> Windows;

        if(XQueryTree(dpy,root,&root,&Parent,&child,&ammountOfChilds))
        {
            for(unsigned int i = 0; i < ammountOfChilds; i++)
            {
                Windows.push_back(child[i]);
            }
            XFree(child);
            std::cout << "OPERATION ENDED" << std::endl;
        }
        else
        {
            std::cout << "ERROR" << std::endl;
        }
        return Windows;
    }
}