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

#include "gui.h"
#include "kmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <FL/fl_utf8.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_File_Chooser.H>
#include <font_selector.h>
#include <X11/Xlocale.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>
#include <X11/Xmd.h>

extern XIM fl_xim_im;
extern XIC fl_xim_ic;
extern Window im_window;

static void set_modifiers(const char* server) 
{
	Window im_window;
	char *p = strdup(server);
	char buf[80];
	char *ptr = p;
	if (!p) return;
	while (*ptr && *ptr != ':') ptr++;
	if (*ptr == ':') {
		*ptr = 0;
		ptr++;
	}
	im_window = Fl::first_window() ? fl_xid(Fl::first_window()) : 0;
	setlocale(LC_ALL, ptr);
	snprintf(buf, 80, "@im=%s", p);
	XSetLocaleModifiers(buf);
	if (fl_xim_ic) XDestroyIC(fl_xim_ic);
	if (fl_xim_im) XCloseIM(fl_xim_im);
	fl_xim_im = NULL;
	fl_xim_ic = NULL;
	fl_xim_im = XOpenIM(fl_display, NULL, NULL, NULL);
	if (fl_xim_im) {
		fl_xim_ic = XCreateIC(fl_xim_im, XNInputStyle,
				(XIMPreeditNothing | XIMStatusNothing),
				XNClientWindow, im_window,
				XNFocusWindow, im_window,
				NULL);
		if (!fl_xim_ic) {
			XCloseIM(fl_xim_im);
			fl_xim_im = NULL;
			setlocale(LC_ALL, "");
			XSetLocaleModifiers("");
			fl_xim_im = XOpenIM(fl_display, NULL, NULL, NULL);
			if (fl_xim_im) {
				fl_xim_ic = XCreateIC(fl_xim_im, XNInputStyle,
					(XIMPreeditNothing | XIMStatusNothing),
					XNClientWindow, im_window,
					XNFocusWindow, im_window,
					NULL);
			}
			printf("bad guy\n");
			if (fl_xim_ic) XSetICFocus(fl_xim_ic);
		} else {
			XSetICFocus(fl_xim_ic);
		}
	} else {
		fl_xim_im = NULL;
		setlocale(LC_ALL, "");
		XSetLocaleModifiers("");
		fl_xim_im = XOpenIM(fl_display, NULL, NULL, NULL);
		if (fl_xim_im) {
			fl_xim_ic = XCreateIC(fl_xim_im, XNInputStyle,
					(XIMPreeditNothing | XIMStatusNothing),
					XNClientWindow, im_window,
					XNFocusWindow, im_window,
					NULL);
		}
		if (fl_xim_ic) XSetICFocus(fl_xim_ic);
	}
	free(p);
}

static void ch_cb(Fl_Widget* w, void* t) 
{
	((Gui*)t)->choice_cb();
}

static void bu_cb(Fl_Widget* w, void* t) 
{
	((Gui*)t)->setup_cb();
	((Gui*)t)->reload();
}

static const int XEventMask =
ExposureMask|StructureNotifyMask
|KeyPressMask|KeyReleaseMask|KeymapStateMask|FocusChangeMask
|ButtonPressMask|ButtonReleaseMask
|EnterWindowMask|LeaveWindowMask
|PointerMotionMask|ClientMessage;

Gui::Gui(int W, int H) : Fl_Window (10, 10, W, H)
{
	dragging = 0;
	//set_flag(256|16);
	in_setup = 0;
	label("InterXim");
	for (int i = 0; i < 12; i++) {
		nb_seq[i] = 0;
		sequence[i] = NULL;
		converter[i] = NULL;
		for (int ii = 0; ii < 5; ii++) {
			multi_offset[i][ii] = 0;
		}
	}
	get_preferences();

	bu = new Fl_Button(2, 2, W/2 - 4, 25, "Setup");
	bu->callback(bu_cb);
	bu->user_data((void*)this);

	create_menu();
	ch = new Fl_Choice(W/2 + 2, 2, W/2 - 4, 25);
	ch->callback(ch_cb);
	ch->user_data((void*)this);
	ch->menu(ch_menu);
	ch->value(conv_nb);

	in = new Fl_Input(0, H - font_size - 8, W, font_size + 8);
	Fl::set_font((Fl_Font)(FL_FREE_FONT + 2), font);
	in->textfont((Fl_Font)(FL_FREE_FONT + 2));
	in->textsize(font_size);
	len = 0;
	start = 0;

	for(int i = 0; i < 12; i++) {
		load_input(i, input[i]);
	}
	end();
	if (!nb_seq[conv_nb]) set_modifiers(ch_menu[conv_nb].text);
	else set_modifiers("");
}

Gui::~Gui()
{

}

void Gui::reload()
{
	get_preferences();
	in->resize (0, h() - font_size - 8, w(), font_size + 8);
	Fl::set_font((Fl_Font)(FL_FREE_FONT + 2), font);
	in->textfont((Fl_Font)(FL_FREE_FONT + 2));
	in->textsize(font_size);
	for(int i = 0; i < 12; i++) {
		load_input(i, input[i]);
	}
}

void Gui::choice_cb()
{
	conv_nb = ch->value();
	if (conv_nb > 11 || conv_nb < 0) conv_nb = 0;
	Fl_Preferences p(Fl_Preferences::USER, "interxim", "options");
	p.set("DefaultInput", conv_nb);
	if (!nb_seq[conv_nb]) set_modifiers(ch_menu[conv_nb].text);
	else set_modifiers("");
}

unsigned short *Gui::get_unicode_buffer(void)
{
	static unsigned short *b = NULL;
	static int bl = 0;
	len = 0;
	if (!in->value()) return NULL;
	len = strlen(in->value());
	if (bl < len) {
		bl = len;
		b = (unsigned short*) realloc(b, sizeof(short) * bl);
	}
	len = fl_utf2unicode((unsigned char*)in->value(), len, b);
	return b;
}

int Gui::reset_unicode_buffer(void)
{
	in->value("");
	start = 0;
	return len;
}

void Gui::key(int k)
{
	Fl::e_keysym = k;
	ch->handle(FL_SHORTCUT);
	redraw();
}

void Gui::get_preferences()
{
	Fl_Preferences p(Fl_Preferences::USER, "interxim", "options");
	p.get("DefaultInput", conv_nb, 0);
	p.get("FontSize", font_size, 16);
	p.get("FontSet", font, 
		"-*-courier-medium-r-normal--*-iso8859-1,"
		"-*-*-*-*-*--*-*-*-*-*-*-iso10646-1"
		, 1023);
	p.get("Input_01", input[0], "None 1", 1023);
	p.get("Input_02", input[1], "None 2", 1023);
	p.get("Input_03", input[2], "None 3", 1023);
	p.get("Input_04", input[3], "None 4", 1023);
	p.get("Input_05", input[4], "None 5", 1023);
	p.get("Input_06", input[5], "None 6"/*"utf-8:en_GB"*/, 1023);
	p.get("Input_07", input[6], "None 7"/*"utf-8:hu_HU"*/, 1023);
	p.get("Input_08", input[7], "None 8"/*"kinput2:ja_JP.eucJP"*/, 1023);
	p.get("Input_09", input[8], "None 9"/*"xcin:zh_TW.big5"*/, 1023);
	p.get("Input_10", input[9], "None 10"/*"Ami:ko_KR"*/, 1023);
	p.get("Input_11", input[10], "None 11"
		/*"xcin-zh_CN.GB2312:zh_CN.GB231"*/, 1023);
	p.get("Input_12", input[11], PKGDATADIR "/Unicode.kmap", 1023);

	p.get("RightToLeftDrection_01", rtl[0], 0);
	p.get("RightToLeftDrection_02", rtl[1], 0);
	p.get("RightToLeftDrection_03", rtl[2], 0);
	p.get("RightToLeftDrection_04", rtl[3], 0);
	p.get("RightToLeftDrection_05", rtl[4], 0);
	p.get("RightToLeftDrection_06", rtl[5], 0);
	p.get("RightToLeftDrection_07", rtl[6], 0);
	p.get("RightToLeftDrection_08", rtl[7], 0);
	p.get("RightToLeftDrection_09", rtl[8], 0);
	p.get("RightToLeftDrection_10", rtl[9], 0);
	p.get("RightToLeftDrection_11", rtl[10], 0);
	p.get("RightToLeftDrection_12", rtl[11], 0);
}

char *Gui::get_short_name(const char *c)
{
	static char buf[80];
	const char *p = "None";
	int l;
	const char *q;
	if (c) {
		p = c + strlen(c);
		while (p > c) {
			p--;
			if (*p == '/') {
				p++;
				break;
			}
		}
	}
	q = p;
	while (*q && *q != '.') q++;
	l = q - p; 
	if (l > 20) l = 20;
	snprintf(buf, l + 1, "%s", p );
	return buf;
}

void Gui::create_menu()
{
	int i = 0;
	ch_menu = new Fl_Menu_Item[13];
	for (; i < 12;i++){
		ch_menu[i].text = strdup(get_short_name(input[i]));
		ch_menu[i].shortcut_ =  FL_F + i + 1;
		ch_menu[i].callback_ = NULL;
		ch_menu[i].user_data_ = NULL;
		ch_menu[i].flags = 0;
		ch_menu[i].labeltype_ = 0;
		ch_menu[i].labelfont_ = 0;
		ch_menu[i].labelsize_ = 0;
		ch_menu[i].labelcolor_  = 0;	
	}
	ch_menu[i].text = NULL;
}

void Gui::convert()
{
	int l, p, nb_matches, rem;
	const char *b;
	const char *s;
	if (!in->value()) return;
	p = in->position();
	b = in->value();
	l = strlen(in->value());

	if (start >= l) {
		start = l;
		return;
	} 
	if (start > p) {
		start = p;
		return ;
	}
	nb_matches = get_matches(b+start, p-start, &s, &rem);
	if (nb_matches == 1) {
		in->replace(start, p - rem, s, strlen(s));
		start += strlen(s);
		in->position(start + rem);
	} else if (nb_matches == 0) {
		start++;
	}
}

int Gui::get_matches(const char *b, int l, const char **r, int *rem)
{
	int nb, n;
	int nbm, mat, partial, found;
	int end, start;
	int off = 0;
	int matches[6];

	if (nb_seq[conv_nb] < 1) return 0;
	char **s = sequence[conv_nb];
	if (l < 1) return 0;

	nb = nb_seq[conv_nb];
	mat = 0;
	*r = NULL;

	for (n = 0; n < 5; n++) {
		start = multi_offset[conv_nb][n];
		if (n < 4 && multi_offset[conv_nb][n + 1] > 0) {
			end = multi_offset[conv_nb][n + 1];
		} else {
			end = nb;
		}
		nbm = 0;
		found = partial = -1;
		// printf("%s %d %d %d\n", b, l, end, nb);
		for(int i = start; i < end; i++) {
			if (s[i][0] == b[off]) {
				if ((l - off < 1 || 
					!strncmp(b + off, s[i], l - off))) 
				{
					if ((int)strlen(s[i]) == l - off) {
						/* exact match */
						found = i;
					}
					candidate[nbm] = i;
					nbm++;
					if (nbm > 10) return 11;
				} else if (nbm < 1 &&
					!strncmp(b + off, s[i],strlen(s[i]))) 
				{
					partial = i;
				}
			}
		}
		if (found >= 0) {
			matches[mat] = found;
			mat++;
		} else if (partial >= 0) {
			matches[mat] = partial;
			mat++;
			if (nbm < 1) nbm = 1;
		} 
		if (end == nb || nbm != 1) {
			break;
		} else if (mat == 0) {
			matches[mat] = candidate[0];
		}
		off += strlen(s[matches[mat - 1]]);
	}
	if (nbm == 1) {
		if (mat > 1) {
			static char buf[80];
			unsigned int ucs = 0, v = 0;;
			while (mat > 0) {
				int r;
				mat--;
				l -= strlen(s[matches[mat]]);
				r = fl_utf2ucs((unsigned char*)
				      converter[conv_nb][matches[mat]],
				      strlen(converter[conv_nb][matches[mat]]),
				      &ucs);
				if (r < 1) ucs = 0;
				v += ucs;			
			}
			buf[fl_ucs2utf(v, buf)] = 0;
			*rem = l;
			*r = buf;
		} else if (mat == 1) {
			*rem = l - strlen(s[matches[0]]);
			*r = converter[conv_nb][matches[0]];
		} else {
			return 11;
		}
		return 1;
	} else if (nbm == 0 && n > 0 && !b[off]) {
		return 11;
	}
	return nbm;
}


int Gui::handle(int e)
{
	int ret;
	static int sx=0, sy=0;

	if (!in_setup && !dragging) Fl::focus_ = in;
	if (e == FL_KEYDOWN) {
		ret = in->handle(FL_KEYDOWN);
		if (Fl::e_keysym != 0) {
			start = in->position();
		} else {
			convert();
		}
		return ret;
		in->redraw();
	}
	ret = Fl_Window::handle(e);
	if ((Fl::belowmouse() != bu && Fl::belowmouse() != ch)  && e == FL_PUSH)
	{
		dragging = 1;
		sx = Fl::e_x;
		sy = Fl::e_y;
		ret = 1;
		Fl::belowmouse(this);
		Fl::pushed(this);
		Fl::focus(this);;
	}
	if (dragging) {
		if (!(Fl::e_state&FL_BUTTON1)) {
			dragging = 0;
			save_position(x_root(), y_root());
		} else if (e == FL_MOVE || e == FL_DRAG) {
			position(Fl::e_x_root - sx, Fl::e_y_root - sy);
			show();
			Fl::check();
			Fl::flush();
		}
	} 
	return 1;
}

void Gui::create_unicode_converter(int n)
{
	int i = nb_seq[n];
	char **c = converter[n];
	char **s = sequence[n];

	while (i > 0) {
		i--;
		free(s[i]);
		free(c[i]);
	}
	for (i = 0; i < 5; i++) {
		multi_offset[n][i] = 0;
	}
	sequence[n] = (char**)realloc(sequence[n], sizeof(char*) * 0x10000);
	converter[n] = (char**)realloc(converter[n], sizeof(char*) * 0x10000);
	c = converter[n];
	s = sequence[n];
	if (!s || !c) return;
	nb_seq[n] = 0x10000;
	char buf[200];
	for(i = 0; i < 0x10000; i++) {
		sprintf(buf, "u%04d", i);
		s[i] = strdup(buf);
		buf[fl_ucs2utf(i, buf)] = 0;
		c[i] = strdup(buf);
	}
}

void Gui::get_position(int* x, int* y)
{
	Fl_Preferences p(Fl_Preferences::USER, "interxim", "options");
	p.get("X", *x, 300);
	p.get("Y", *y, 300);
}

void Gui::save_position(int x, int y)
{
	Fl_Preferences p(Fl_Preferences::USER, "interxim", "options");
	p.set("X", x);
	p.set("Y", y);
}

void Gui::setup_cb()
{
	XLowerWindow(fl_display, fl_xid(this));
	Fl_Window *win = new Fl_Window(400, 385);
	win->label("InterXim Setup");
	int X = 2;
	int Y = 2;
	Fl_Button *ok, *cancel;
	Fl_Button *sel[12];
	Fl_Input *conv[12];
	Fl_Button *dir[12];
	Fl_Button *fnt;
	Fl_Input *fnt_size;
	char buf[80];

	in_setup = 1;
	for (int i = 0; i < 12;i++) {
		sel[i] = new Fl_Button(X, Y, 25, 25);
		sel[i]->label("@2>[]");
		conv[i] = new Fl_Input(X + 27, Y, 340, 25);
		conv[i]->value(input[i]);
		conv[i]->when(FL_WHEN_CHANGED);
		dir[i] = new Fl_Button(400 - 27, Y, 25, 25);
		if (rtl[i]) {
			dir[i]->label("@4->");
		} else {
			dir[i]->label("@->");
		}

		Y += 27;
	}
	fnt = new Fl_Button(X + 40, Y, 60, 25);
	fnt->label("Font");
	fnt_size = new Fl_Input(X + 200, Y, 50, 25);
	fnt_size->label("Font size ");
	sprintf(buf, "%d", font_size);
	fnt_size->value(buf);
	fnt_size->when(FL_WHEN_CHANGED);

	Y += 27;
	Y += 2;
	ok  = new Fl_Button(5, Y, 190, 25);
	ok->label("Ok");
	cancel =  new Fl_Button(205, Y, 190, 25);
	cancel->label("Cancel");
	win->end();
	win->set_modal();
	win->show();
	for (;;) {
		Fl_Widget *o = Fl::readqueue();

		for (int i = 0; i < 12; i++) {
			if (o == sel[i]) {
				const char *f;
				f  = fl_file_chooser("Keyboard Map", "*.kmap",
					PKGDATADIR "/");
				if (f) {
					conv[i]->value(f);
				}
			} else if (o == conv[i]) {

			} else if (o == dir[i]) {
				if (!strcmp(dir[i]->label(), "@->")) {
					dir[i]->label("@4->");
				} else {
					dir[i]->label("@->");
				}
			}
		}
		if (o == fnt) {
			const char *f;
			f = font_selector(font);
			if (f) {
				strncpy(font, f, 1024);
				font[1023] = 0;
			}
		} else if (o == ok) {
			int val;
			Fl_Preferences p(Fl_Preferences::USER, 
				"interxim", "options");
			val = atoi(fnt_size->value());
			if (val < 8) val = 16;
			p.set("FontSize", val);
			p.set("FontSet", font);
			
			for (int i = 0; i < 12; i++) {
				char buf[80];
				sprintf(buf, "Input_%02d", i + 1);
				p.set(buf, conv[i]->value());
				sprintf(buf, "RightToLeftDrection_%02d", i + 1);
				if (!strcmp(dir[i]->label(), "@->")) {
					val = 0;
				} else {
					val = 1;
				}
				p.set(buf, val);
			}
			break;
		} else if (o == win || o == cancel) {
			break;
		}
		if (!o) {
			Fl::wait();
		}
	}
	win->hide();
	delete(win);
	Fl::flush();
	in_setup = 0;
	XRaiseWindow(fl_display, fl_xid(this));
}

void Gui::load_input(int n, const char *f)
{
	int l = nb_seq[n];
	while (l > 0) {
		l--;
		free(sequence[n][l]);
		free(converter[n][l]);
	}
	free(sequence[n]);
	free(converter[n]);
	for (int ii = 0; ii < 5; ii++) {
		multi_offset[n][ii] = 0;
	}
	nb_seq[n] = 0;
	free((char*)ch_menu[n].text);
	if (strstr(f, ".kmap")) {
		ch_menu[n].text = strdup(get_short_name(f));
	} else {
		ch_menu[n].text = strdup(f);
	}
	nb_seq[n] = load_kmap(f, &sequence[n], &converter[n], 
			multi_offset[n]);
}

