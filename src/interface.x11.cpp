/******************************************************************************
 *
 *                 Copyright (c) 2000  O'ksi'D
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 *
 *   Author : Jean-Marc Lienher ( http://oksid.ch )
 *
 ******************************************************************************/

#include <stdio.h>
#include "interface.h"

int IxmMainLoop(Display *dpy, Window w)
{
    for (;;) {
        XEvent event;
        XNextEvent(dpy, &event);
        if (XFilterEvent(&event, None) == True) {
                //fprintf(stderr, "window %ld\n",event.xany.window);
                continue;
        }
    }

	return 0;
}

Display *IxmOpenDisplay(const char *name)
{
	return XOpenDisplay(name);
}

Window IxmCreateSimpleWindow(Display *dpy)
{
	return XCreateSimpleWindow(dpy, DefaultRootWindow(dpy),
			0, 700, 400, 800-700,
			0, WhitePixel(dpy, DefaultScreen(dpy)),
			WhitePixel(dpy, DefaultScreen(dpy)));
}


int IxmGetUnicodeBuffer( unsigned short **ret)
{
	static unsigned short b[] = {'h', 'e', 'l','l','o', 0};
	*ret = b; 
	return 5;
}

void IxmHandleKex(KeySym key, const char *txt, int len)
{
	printf("XXXXXXXXXXXX %d %s\n", key, txt);
}


