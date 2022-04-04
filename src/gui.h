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

#ifndef gui_h
#define gui_h

#include <FL/Fl.H>
#include <FL/x.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>


class Gui : public Fl_Window
{
public:
	char font[1024];
	int font_size;
	Fl_Input *in;
	Fl_Menu_Item * ch_menu;
	Fl_Choice *ch;
	Fl_Button *bu;
	int len;
	int start;
	int conv_nb;
	char **sequence[12];
	char **converter[12];
	int nb_seq[12];
	int candidate[11];
	int multi_offset[12][5];
	char input[12][1024];
	int rtl[12];
	int in_setup;
	int dragging;

	Gui(int W, int H);
	~Gui();
	void get_preferences();
	unsigned short *get_unicode_buffer(void);
	int reset_unicode_buffer(void);
	void key(int);
	int handle(int e);
	void create_menu(void);
	void choice_cb(void);
	void convert(void);
	int get_matches(const char *b, int l, const char **r, int *rem);
	void create_unicode_converter(int n);
	void get_position(int* x, int* y);
	void save_position(int x, int y);
	void setup_cb(void);
	void load_input(int n, const char *f);
	char *get_short_name(const char*c);
	void reload(void);
};
#endif


