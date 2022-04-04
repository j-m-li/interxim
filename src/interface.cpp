/******************************************************************************
 *
 *                 Copyright (c) 2002  O'ksi'D
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

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Window.H>
#include <stdio.h>
#include "interface.h"
#include "gui.h"

extern int fl_handle(const XEvent &xevent);

static Gui *gui = NULL;
static int gui_x = 300;
static int gui_y = 380;

extern Fl_Window* fl_xmousewin;

int IxmMainLoop(Display *dpy, Window w)
{
	for(;;) {
		XEvent event;
		XNextEvent(dpy, &event);
		if (event.xany.window == w /*|| (gui && gui->shown() && 
			event.xany.window != fl_xid(gui))*/) 
		{
			 XFilterEvent(&event, None);
		} else {
			fl_handle(event);
			if (gui && event.type == MotionNotify) {
			
				gui->handle(FL_MOVE);
			}
			Fl::wait(0.0);
		}
	}
	return 0;
}

extern "C" {
  static int io_error_handler(Display*) {
    Fl::fatal("X I/O error");
    return 0;
  }

  static int xerror_handler(Display* d, XErrorEvent* e) {
    char buf1[128], buf2[128];
    sprintf(buf1, "XRequest.%d", e->request_code);
    XGetErrorDatabaseText(d,"",buf1,buf1,buf2,128);
    XGetErrorText(d, e->error_code, buf1, 128);
    Fl::warning("%s: %s 0x%lx", buf2, buf1, e->resourceid);
    return 0;
  }
}

extern XIM fl_xim_im;

Display *IxmOpenDisplay(const char *name)
{
	XSetIOErrorHandler(io_error_handler);
	XSetErrorHandler(xerror_handler);
	fl_xim_im = (XIM)1;
	Display *d = XOpenDisplay(name);
	fl_open_display(d);
	fl_xim_im = 0;
	return fl_display;
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
	*ret = gui->get_unicode_buffer(); 
	return gui->reset_unicode_buffer();
}

void IxmHandleKex(KeySym key, const char *txt, int len)
{
	if (!gui) return;
	if (key >= XK_F1 && key <= XK_F12) {
		gui->key((int)(FL_F + 1 + key - XK_F1));
	} else if (key >= 0xff51 && key <= 0xff54) {
	      static const unsigned short table[4] = {
        	   FL_Left, FL_Up, FL_Right, FL_Down};
         	Fl::e_keysym  = (int) table[key-0xff51];
		Fl::e_length = 0;
		gui->handle(FL_KEYDOWN);
		Fl::e_keysym = 0;
	} else {
		Fl::e_keysym = 0;
		Fl::e_text = (char*)txt;
		Fl::e_length = len;
		gui->handle(FL_KEYDOWN);
	}
}

void IxmShow(void)
{
	static char *n = "InterXim";
	XWMHints wm_hints;
	static XTextProperty name;
	XSetWindowAttributes win_attr;
	if (!gui) {
		gui = new Gui(300, 80);
		XStringListToTextProperty(&n, 1, &name);
	}
	gui->get_position(&gui_x, &gui_y);
	gui->position(gui_x, gui_y);
	gui->show();
	XUnmapWindow(fl_display,  fl_xid(gui));
	win_attr.override_redirect = True;
	wm_hints.flags = InputHint;
	wm_hints.input = False;
	XSetWMProperties(fl_display, fl_xid(gui), &name,
			&name, NULL, 0, NULL, &wm_hints, NULL);
	XMapWindow(fl_display,  fl_xid(gui));
}

void IxmHide(void)
{
	if (!gui) return;
	gui->hide();
}

void IxmLower(void)
{
	if (!gui) return;
	//gui->show();
	if (gui->shown() && !gui->in_setup) {
		XLowerWindow(fl_display, fl_xid(gui));
	}
	Fl::check();
	Fl::flush();
}

