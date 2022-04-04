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

#ifndef interface_h
#define interface_h

#include <X11/Xlocale.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

int IxmMainLoop(Display *dpy, Window w);
Display *IxmOpenDisplay(const char *name);
Window IxmCreateSimpleWindow(Display *dpy);
int IxmGetUnicodeBuffer(unsigned short **ret);
void IxmHandleKex(KeySym key, const char *txt, int len);
void IxmShow(void);
void IxmHide(void);
void IxmLower(void);
#endif


