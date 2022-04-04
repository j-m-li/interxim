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

#include "kmap.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <FL/fl_utf8.H>
#include <unistd.h>
#include <sys/types.h>

enum {
	IN_QUOTE	= 0x0001,
	IN_COMMENT	= 0x0002,
	IN_AFTER_E	= 0x0004,
	IN_ESCAPE	= 0x0008,
	IN_HEX		= 0x0010,
	IN_DEC		= 0x0020,
	IN_OCT		= 0x0040,
	IN_HEX_OCT	= 0x0080,
};

int from_hex(char p) 
{
	if (p >= '0' && p <= '9') {
		return p - '0';
	} else if (p >= 'A' && p <= 'F') {
		return p - 'A' + 10;
	} else if (p >= 'a' && p <= 'f') {
		return p - 'a' + 10;
	}
	return 16;
}

void clean(char *bus, int *posi, char p, int *state, int *ucs) 
{
	int b = *posi;
	int st = *state;
	int u = *ucs;
	int was_escape = st & IN_ESCAPE;
	
	if (st & IN_HEX_OCT) {
		st &= ~IN_HEX_OCT;
		if (from_hex(p) < 8) {
			u = from_hex(p);
			st |= IN_OCT;
		} else if (p == 'x' || p == 'X') {
			u = 0;
			st |= IN_HEX;
		} else {
			bus[b] = '0';
			b++;
		}
	} else if (st & IN_HEX) {
		int v = from_hex(p);
		if (v < 16) {
			u *= 16;
			u += v;
		} else {
			st &= ~IN_HEX;
			b += fl_ucs2utf((unsigned int)u, bus + b);
		}
	} else if (st & IN_OCT) {
		int v = from_hex(p);
		if (v < 8) {
			u *= 8;
			u += v;
		} else {
			st &= ~IN_OCT;
			b += fl_ucs2utf((unsigned int)u, bus + b);
		}
	} else if (st & IN_DEC) {
		int v = from_hex(p);
		if (v < 10) {
			u *= 10;
			u += v;
		} else {
			st &= ~IN_DEC;
			b += fl_ucs2utf((unsigned int)u, bus + b);
		}
	} else if (st & IN_ESCAPE) {
		u = 0;
		st &= ~IN_ESCAPE;
		if (p == '0') {
			st |= IN_OCT;
		} else if (p == 'x' || p == 'X') {
			st |= IN_HEX;
		} else if (from_hex(p) < 16) {
			st |= IN_DEC;
			u = from_hex(p);
		} else {
			bus[b] = p;
			b++;
		}
	}
	if (!(st & (IN_HEX|IN_OCT|IN_DEC)) && !was_escape) {
		if (p == '"') {
			st = 0;
			bus[b] = 0;
		} else if (p == '=') {
			st &= ~IN_QUOTE;
			st |= IN_AFTER_E;
			bus[b] = 0;
		} else if (p == '0') {
			st |= IN_HEX_OCT;
		} else if (from_hex(p) < 10) {
			st |= IN_DEC;
			u = from_hex(p);
		} else if (p == '\\') {
			st |= IN_ESCAPE;
		} else if (p != ' ') {
			bus[b] = p;
			b++;
		}
	}

	*posi = b;
	*state = st;
	*ucs = u;
}

int get_line(char *buf, int len, char **s, char **c)
{
	char *p;
	char * end;
	int st = 0;
	static char before[1024];
	static char after[1024];
	int b = 0;
	int a = 0;
	int ucs;
	p = buf;
	end = buf + len;
	before[0] = 0;
	after[0] = 0;
	while (p < end) {
		if (*p == '\n') {
			p++;
			break;
		}

		if (st & IN_QUOTE)  {
			clean(before, &b, *p, &st, &ucs);
		} else if (st & IN_AFTER_E) {
			clean(after, &a, *p, &st, &ucs);
		} else if (st & IN_COMMENT) {
			if (*p == '\n') st &= ~IN_COMMENT;
		} else {
			if (*p == '"') {
				st |= IN_QUOTE;
			} else if (*p == '/') {
				st |= IN_COMMENT;
			}
		}
		p++;
	}
	if (*(p-1) != '\n') {
		return 0;
	}
	*s = before;
	*c = after;
	return p - buf;

}

int load_kmap(const char *f, char ***seq, char ***conv, int offs[5])
{
	int fd;
	char **sequence;
	char **convert;	
	int nbl = 0;
	int ret;
	char buf[1024];
	int i;
	char *tmp = NULL;
	int eat;
	char *c;
	char *s;
	char *p;
	int mlen = 0;
	int o = 0;
	int multi = 0;

	sequence = *seq = NULL;
	convert = *conv = NULL;
	offs[0] = 0; offs[1] = 0; offs[2] = 0; offs[3] = 0; offs[4] = 0;
	fd = open(f, O_RDONLY);
	if (fd < 1) return 0;
	i = 0;
	ret = read(fd, buf, 1024);
	for(;ret > 0;) {
		p = buf;
		eat = get_line(p, ret,  &s, &c);
		while (eat) {
			if (!multi && nbl == 0 && strstr(s, "+") && c[0] == 0) {
				multi = 1;
			} else if (multi && !c[0] && strstr(s, "begin")) {
				if (o < 5) offs[o] = nbl;
				o++;
			} else if (multi && !c[0] && strstr(s, "end")) {
				;
			} else if (s[0] || c[0]) {
				if (mlen < nbl + 1) {
					if (mlen < 16) mlen = 16;
					mlen *= 2;
					sequence = (char**) realloc(
						sequence, sizeof(char*)*mlen);
					convert = (char**) realloc(
						convert, sizeof(char*)*mlen);
				}
				sequence[nbl] = strdup(s);
				convert[nbl] = strdup(c);
				nbl++;
			}
				
			ret -= eat;
			p += eat;
			eat = get_line(p, ret, &s, &c);
		} 
		lseek(fd, -ret, SEEK_CUR);
		ret = read(fd, buf, 1024);
	}
	if (tmp) free(tmp);
	close(fd);
	*seq = sequence;
	*conv = convert;
	return nbl;
}
