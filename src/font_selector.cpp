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

#include "font_selector.h"
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Box.H>
#include <string.h>
#include <stdlib.h>
#define _(x) x

class FontDisplay : public Fl_Widget {
        void draw();
public:
        int font, size;
        FontDisplay(Fl_Boxtype B, int X, int Y, int W, int H,
                const char* L = 0) : Fl_Widget(X,Y,W,H,L)
        {
                box(B);
                font = 0;
                size = 12;
        }
};


void FontDisplay::draw()
{
        draw_box();
        fl_font((Fl_Font)font, size);
        fl_color(FL_BLACK);
        fl_draw(label(), x()+3, y()+3, w()-6, h()-6, align());
}

static Fl_Window *form = NULL;
static FontDisplay *textobj;
static Fl_Hold_Browser *fontobj;
static Fl_Button *ok, *cancel;
static char label[1024];

static void font_cb(Fl_Widget *, long)
{
        int fn = fontobj->value();
        if (!fn) return;
        fn += FL_FREE_FONT - 1;
        textobj->font = fn;
        textobj->redraw();
}

static Fl_Window *create_selector_form()
{
        int i = 0;
        uchar c;

        if (form) return form;
        form = new Fl_Window(550,370);
        form->label(_("Font Selection"));

        strcpy(label, "Hello, world!\n");

        for (c = ' '+1; c < 127; c++) {
                if (!(c&0x1f)) label[i++]='\n';
                label[i++]=c;
        }
        label[i++] = '\n';
        for (c = 0xA1; c; c++) {
                if (!(c&0x1f)) label[i++]='\n';
                i += fl_ucs2utf(c, label + i);
        }
        label[i] = 0;
        textobj = new FontDisplay(FL_FRAME_BOX,10,10,530,170,label);
        textobj->align(FL_ALIGN_TOP|FL_ALIGN_LEFT|FL_ALIGN_INSIDE|
                FL_ALIGN_CLIP);
        fontobj = new Fl_Hold_Browser(10, 190, 390, 170);
        fontobj->box(FL_FRAME_BOX);
        fontobj->callback(font_cb);
        form->resizable(fontobj);

        ok = new Fl_Button(410, 250, 130, 25);
        ok->label(_("OK"));

        cancel = new  Fl_Button(410, 280, 130, 25);
        cancel->label(_("Cancel"));

        form->end();
        form->set_modal();

        int k = Fl::set_fonts("-*");
        for (i = FL_FREE_FONT; i < k; i++) {
                int t; const char *name = Fl::get_font_name((Fl_Font)i,&t);
                char buffer[128];
                if (t) {
                        char *p = buffer;
                        if (t & FL_BOLD) {*p++ = '@'; *p++ = 'b';}
                        if (t & FL_ITALIC) {*p++ = '@'; *p++ = 'i';}
                        strcpy(p,name);
                        name = buffer;
                }
                fontobj->add(name);
        }
        fontobj->value(1);
        font_cb(fontobj,0);
        return form;
}

static const char * font_selector()
{
        int r;

        create_selector_form();

        form->show();
        for (;;) {
                Fl_Widget *o = Fl::readqueue();
                if (!o) Fl::wait();
                else if (o == cancel) {r = 0; break;}
                else if (o == ok) {r = 1; break;}
                else if (o == form) {r = 0; break;}
        }
        form->hide();
        Fl::check();
        Fl::redraw();
        if (r == 0) return NULL;
        return Fl::get_font((Fl_Font)textobj->font);
}


char *font_selector(const char *f)
{
        Fl_Button *ok, *cancel, *add, *remove;
        Fl_Hold_Browser *fontobj;
        Fl_Window *win;

        win = new Fl_Window(400, 300);
        win->label(_("Font set selection"));

        fontobj = new Fl_Hold_Browser(5, 5, 390, 200);
        fontobj->box(FL_FRAME_BOX);
	if (f) {
		char *l = strdup(f);
                char *ptr = l;
                char *ll = l;
                while (*ptr) {
                        if (*ptr == ',') {
                                *ptr = '\0';
                                fontobj->add(l);
                                l = ptr + 1;
                        }
                        ptr++;
                }
                if (*l) fontobj->add(l);
                free(ll);
        }

        ok  = new Fl_Button(5, 270, 190, 25);
        ok->label("OK");

        cancel  = new Fl_Button(205, 270, 190, 25);
        cancel->label("Cancel");

        add  = new Fl_Button(5, 230, 190, 25);
        add->label("Add font");

        remove  = new Fl_Button(205, 230, 190, 25);
        remove->label("Remove font");

        win->end();
        win->set_modal();
        win->show();
	f = NULL;
        for (;;) {
                Fl_Widget *o = Fl::readqueue();
                if (!o) Fl::wait();
                else if (o == ok) {
                        static char buf[2048];
                        int s = fontobj->size() + 1;
                        int i = 1;

                        buf[0] = '\0';
                        while (i < s) {
                                const char *b = fontobj->text(i);
                                if (b) {
                                        if (i > 1) strcat(buf, ",");
                                        strcat(buf, b);
                                }
                                i++;
                        }
			f = buf;
			break;
                }
                else if (o == remove && fontobj->value()) {
                        fontobj->remove(fontobj->value());
                }
                else if (o == add) {
                        const char *s;
                        s = font_selector();
                        if (s) fontobj->add(s);
                }
                else if (o == win) {break;}
                else if (o == cancel) {break;}
        }
        win->hide();
        delete(ok);
        delete(cancel);
        delete(add);
        delete(remove);
        delete(fontobj);
        delete(win);
        Fl::flush();
	return (char*)f;
}
	

