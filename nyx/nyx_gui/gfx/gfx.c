/*
 * Copyright (c) 2018 naehrwert
 * Copyright (c) 2018-2022 CTCaer
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdarg.h>
#include <string.h>
#include "gfx.h"

// Global gfx console and context.
gfx_ctxt_t gfx_ctxt;
gfx_con_t gfx_con;

static bool gfx_con_init_done = false;

static const u8 _gfx_font[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Char 032 ( )
	0x00, 0x30, 0x30, 0x18, 0x18, 0x00, 0x0C, 0x00, // Char 033 (!)
	0x00, 0x22, 0x22, 0x22, 0x00, 0x00, 0x00, 0x00, // Char 034 (")
	0x00, 0x66, 0x66, 0xFF, 0x66, 0xFF, 0x66, 0x66, // Char 035 (#)
	0x00, 0x18, 0x7C, 0x06, 0x3C, 0x60, 0x3E, 0x18, // Char 036 ($)
	0x00, 0x46, 0x66, 0x30, 0x18, 0x0C, 0x66, 0x62, // Char 037 (%)
	0x00, 0x3C, 0x66, 0x3C, 0x1C, 0xE6, 0x66, 0xFC, // Char 038 (&)
	0x00, 0x18, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00, // Char 039 (')
	0x00, 0x30, 0x18, 0x0C, 0x0C, 0x18, 0x30, 0x00, // Char 040 (()
	0x00, 0x0C, 0x18, 0x30, 0x30, 0x18, 0x0C, 0x00, // Char 041 ())
	0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00, // Char 042 (*)
	0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00, // Char 043 (+)
	0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x0C, 0x00, // Char 044 (,)
	0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x00, // Char 045 (-)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, // Char 046 (.)
	0x00, 0x40, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x00, // Char 047 (/)
	0x00, 0x3C, 0x66, 0x76, 0x6E, 0x66, 0x3C, 0x00, // Char 048 (0)
	0x00, 0x18, 0x1C, 0x18, 0x18, 0x18, 0x7E, 0x00, // Char 049 (1)
	0x00, 0x3C, 0x62, 0x30, 0x0C, 0x06, 0x7E, 0x00, // Char 050 (2)
	0x00, 0x3C, 0x62, 0x38, 0x60, 0x66, 0x3C, 0x00, // Char 051 (3)
	0x00, 0x6C, 0x6C, 0x66, 0xFE, 0x60, 0x60, 0x00, // Char 052 (4)
	0x00, 0x7E, 0x06, 0x7E, 0x60, 0x66, 0x3C, 0x00, // Char 053 (5)
	0x00, 0x3C, 0x06, 0x3E, 0x66, 0x66, 0x3C, 0x00, // Char 054 (6)
	0x00, 0x7E, 0x30, 0x30, 0x18, 0x18, 0x18, 0x00, // Char 055 (7)
	0x00, 0x3C, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00, // Char 056 (8)
	0x00, 0x3C, 0x66, 0x7C, 0x60, 0x66, 0x3C, 0x00, // Char 057 (9)
	0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x18, 0x00, // Char 058 (:)
	0x00, 0x00, 0x18, 0x00, 0x18, 0x18, 0x0C, 0x00, // Char 059 (;)
	0x00, 0x70, 0x1C, 0x06, 0x06, 0x1C, 0x70, 0x00, // Char 060 (<)
	0x00, 0x00, 0x3E, 0x00, 0x3E, 0x00, 0x00, 0x00, // Char 061 (=)
	0x00, 0x0E, 0x38, 0x60, 0x60, 0x38, 0x0E, 0x00, // Char 062 (>)
	0x00, 0x3C, 0x66, 0x30, 0x18, 0x00, 0x18, 0x00, // Char 063 (?)
	0x00, 0x3C, 0x66, 0x76, 0x76, 0x06, 0x46, 0x3C, // Char 064 (@)
	0x00, 0x3C, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00, // Char 065 (A)
	0x00, 0x3E, 0x66, 0x3E, 0x66, 0x66, 0x3E, 0x00, // Char 066 (B)
	0x00, 0x3C, 0x66, 0x06, 0x06, 0x66, 0x3C, 0x00, // Char 067 (C)
	0x00, 0x1E, 0x36, 0x66, 0x66, 0x36, 0x1E, 0x00, // Char 068 (D)
	0x00, 0x7E, 0x06, 0x1E, 0x06, 0x06, 0x7E, 0x00, // Char 069 (E)
	0x00, 0x3E, 0x06, 0x1E, 0x06, 0x06, 0x06, 0x00, // Char 070 (F)
	0x00, 0x3C, 0x66, 0x06, 0x76, 0x66, 0x3C, 0x00, // Char 071 (G)
	0x00, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00, // Char 072 (H)
	0x00, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, // Char 073 (I)
	0x00, 0x78, 0x30, 0x30, 0x30, 0x36, 0x1C, 0x00, // Char 074 (J)
	0x00, 0x66, 0x36, 0x1E, 0x1E, 0x36, 0x66, 0x00, // Char 075 (K)
	0x00, 0x06, 0x06, 0x06, 0x06, 0x06, 0x7E, 0x00, // Char 076 (L)
	0x00, 0x46, 0x6E, 0x7E, 0x56, 0x46, 0x46, 0x00, // Char 077 (M)
	0x00, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x66, 0x00, // Char 078 (N)
	0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, // Char 079 (O)
	0x00, 0x3E, 0x66, 0x3E, 0x06, 0x06, 0x06, 0x00, // Char 080 (P)
	0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x70, 0x00, // Char 081 (Q)
	0x00, 0x3E, 0x66, 0x3E, 0x1E, 0x36, 0x66, 0x00, // Char 082 (R)
	0x00, 0x3C, 0x66, 0x0C, 0x30, 0x66, 0x3C, 0x00, // Char 083 (S)
	0x00, 0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00, // Char 084 (T)
	0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00, // Char 085 (U)
	0x00, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, // Char 086 (V)
	0x00, 0x46, 0x46, 0x56, 0x7E, 0x6E, 0x46, 0x00, // Char 087 (W)
	0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00, // Char 088 (X)
	0x00, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00, // Char 089 (Y)
	0x00, 0x7E, 0x30, 0x18, 0x0C, 0x06, 0x7E, 0x00, // Char 090 (Z)
	0x00, 0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, // Char 091 ([)
	0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00, // Char 092 (\)
	0x00, 0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, // Char 093 (])
	0x00, 0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, // Char 094 (^)
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, // Char 095 (_)
	0x00, 0x0C, 0x18, 0x30, 0x00, 0x00, 0x00, 0x00, // Char 096 (`)
	0x00, 0x00, 0x3C, 0x60, 0x7C, 0x66, 0x7C, 0x00, // Char 097 (a)
	0x00, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3E, 0x00, // Char 098 (b)
	0x00, 0x00, 0x3C, 0x06, 0x06, 0x06, 0x3C, 0x00, // Char 099 (c)
	0x00, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C, 0x00, // Char 100 (d)
	0x00, 0x00, 0x3C, 0x66, 0x7E, 0x06, 0x3C, 0x00, // Char 101 (e)
	0x00, 0x38, 0x0C, 0x3E, 0x0C, 0x0C, 0x0C, 0x00, // Char 102 (f)
	0x00, 0x00, 0x7C, 0x66, 0x7C, 0x40, 0x3C, 0x00, // Char 103 (g)
	0x00, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x00, // Char 104 (h)
	0x00, 0x18, 0x00, 0x1C, 0x18, 0x18, 0x3C, 0x00, // Char 105 (i)
	0x00, 0x30, 0x00, 0x30, 0x30, 0x30, 0x1E, 0x00, // Char 106 (j)
	0x00, 0x06, 0x06, 0x36, 0x1E, 0x36, 0x66, 0x00, // Char 107 (k)
	0x00, 0x1C, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x00, // Char 108 (l)
	0x00, 0x00, 0x66, 0xFE, 0xFE, 0xD6, 0xC6, 0x00, // Char 109 (m)
	0x00, 0x00, 0x3E, 0x66, 0x66, 0x66, 0x66, 0x00, // Char 110 (n)
	0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00, // Char 111 (o)
	0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x00, // Char 112 (p)
	0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x00, // Char 113 (q)
	0x00, 0x00, 0x3E, 0x66, 0x06, 0x06, 0x06, 0x00, // Char 114 (r)
	0x00, 0x00, 0x7C, 0x06, 0x3C, 0x60, 0x3E, 0x00, // Char 115 (s)
	0x00, 0x18, 0x7E, 0x18, 0x18, 0x18, 0x70, 0x00, // Char 116 (t)
	0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x7C, 0x00, // Char 117 (u)
	0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00, // Char 118 (v)
	0x00, 0x00, 0xC6, 0xD6, 0xFE, 0x7C, 0x6C, 0x00, // Char 119 (w)
	0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00, // Char 120 (x)
	0x00, 0x00, 0x66, 0x66, 0x7C, 0x60, 0x3C, 0x00, // Char 121 (y)
	0x00, 0x00, 0x7E, 0x30, 0x18, 0x0C, 0x7E, 0x00, // Char 122 (z)
	0x00, 0x18, 0x08, 0x08, 0x04, 0x08, 0x08, 0x18, // Char 123 ({)
	0x00, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, // Char 124 (|)
	0x00, 0x0C, 0x08, 0x08, 0x10, 0x08, 0x08, 0x0C, // Char 125 (})
	0x00, 0x00, 0x00, 0x4C, 0x32, 0x00, 0x00, 0x00  // Char 126 (~)
};

void gfx_clear_grey(u8 color)
{
	memset(gfx_ctxt.fb, color, gfx_ctxt.width * gfx_ctxt.height * 4);
}

void gfx_clear_color(u32 color)
{
	for (u32 i = 0; i < gfx_ctxt.width * gfx_ctxt.height; i++)
		gfx_ctxt.fb[i] = color;
}

void gfx_init_ctxt(u32 *fb, u32 width, u32 height, u32 stride)
{
	gfx_ctxt.fb = fb;
	gfx_ctxt.width = width;
	gfx_ctxt.height = height;
	gfx_ctxt.stride = stride;

	gfx_clear_grey(0);
}

void gfx_con_init()
{
	gfx_con.gfx_ctxt = &gfx_ctxt;
	gfx_con.fntsz = 16;
	gfx_con.x = 0;
	gfx_con.y = 0;
	gfx_con.savedx = 0;
	gfx_con.savedy = 0;
	gfx_con.fgcol = TXT_CLR_DEFAULT;
	gfx_con.fillbg = 1;
	gfx_con.bgcol = TXT_CLR_BG;
	gfx_con.mute = 0;

	gfx_con_init_done = true;
}

void gfx_con_setcol(u32 fgcol, int fillbg, u32 bgcol)
{
	gfx_con.fgcol = fgcol;
	gfx_con.fillbg = fillbg;
	gfx_con.bgcol = bgcol;
}

void gfx_con_getpos(u32 *x, u32 *y)
{
	*x = gfx_con.x;
	*y = gfx_con.y;
}

static int gfx_column = 0;
void gfx_con_setpos(u32 x, u32 y)
{
	gfx_con.x = x;
	gfx_con.y = y;

	if (!x)
		gfx_column = 0;
}

void gfx_putc(char c)
{
	// Duplicate code for performance reasons.
	switch (gfx_con.fntsz)
	{
	case 16:
		if (c >= 32 && c <= 126)
		{
			u8 *cbuf = (u8 *)&_gfx_font[8 * (c - 32)];
			for (u32 i = 0; i < 16; i += 2)
			{
				u8 v = *cbuf;
				for (u32 k = 0; k < 2; k++)
				{
					u32 fb_off = gfx_con.y + i + k + (gfx_ctxt.width - gfx_con.x) * gfx_ctxt.stride;
					for (u32 j = 0; j < 16; j += 2)
					{
						for (u32 l = 0; l < 2; l++)
						{
							if (v & 1)
								gfx_ctxt.fb[fb_off - (j + l) * gfx_ctxt.stride] = gfx_con.fgcol;
							else if (gfx_con.fillbg)
								gfx_ctxt.fb[fb_off - (j + l) * gfx_ctxt.stride] = gfx_con.bgcol;
						}
						v >>= 1;
					}
					v = *cbuf;
				}
				cbuf++;
			}
			gfx_con.x += 16;
			if (gfx_con.x > gfx_ctxt.width - 16)
			{
				gfx_con.x = gfx_column;
				gfx_con.y += 16;
				if (gfx_con.y > gfx_ctxt.height - 33)
				{
					gfx_con.y = 0;

					if (!gfx_column)
						gfx_column = 640;
					else
						gfx_column = 0;
					gfx_con.x = gfx_column;
				}
			}
		}
		else if (c == '\n')
		{
			gfx_con.x = gfx_column;
			gfx_con.y += 16;
			if (gfx_con.y > gfx_ctxt.height - 33)
			{
				gfx_con.y = 0;

				if (!gfx_column)
					gfx_column = 640;
				else
					gfx_column = 0;
				gfx_con.x = gfx_column;
			}
		}
		break;
	case 8:
	default:
		if (c >= 32 && c <= 126)
		{
			u8 *cbuf = (u8 *)&_gfx_font[8 * (c - 32)];
			for (u32 i = 0; i < 8; i++)
			{
				u8 v = *cbuf++;
				u32 fb_off = gfx_con.y + i + (gfx_ctxt.width - gfx_con.x) * gfx_ctxt.stride;
				for (u32 j = 0; j < 8; j++)
				{
					if (v & 1)
						gfx_ctxt.fb[fb_off - (j * gfx_ctxt.stride)] = gfx_con.fgcol;
					else if (gfx_con.fillbg)
						gfx_ctxt.fb[fb_off - (j * gfx_ctxt.stride)] = gfx_con.bgcol;
					v >>= 1;
				}
			}
			gfx_con.x += 8;
			if (gfx_con.x > gfx_ctxt.width / 2 + gfx_column - 8)
			{
				gfx_con.x = gfx_column;
				gfx_con.y += 8;
				if (gfx_con.y > gfx_ctxt.height - 33)
				{
					gfx_con.y = 0;

					if (!gfx_column)
						gfx_column = 640;
					else
						gfx_column = 0;
					gfx_con.x = gfx_column;
				}
			}
		}
		else if (c == '\n')
		{
			gfx_con.x = gfx_column;
			gfx_con.y += 8;
			if (gfx_con.y > gfx_ctxt.height - 33)
			{
				gfx_con.y = 0;

				if (!gfx_column)
					gfx_column = 640;
				else
					gfx_column = 0;
				gfx_con.x = gfx_column;
			}
		}
		break;
	}
}

void gfx_puts(const char *s)
{
	if (!s || !gfx_con_init_done || gfx_con.mute)
		return;

	for (; *s; s++)
		gfx_putc(*s);
}

static void _gfx_putn(u32 v, int base, char fill, int fcnt)
{
	static const char digits[] = "0123456789ABCDEF";

	char *p;
	char buf[65];
	int c = fcnt;
	bool negative = false;

	if (base != 10 && base != 16)
		return;

	// Account for negative numbers.
	if (base == 10 && v & 0x80000000)
	{
		negative = true;
		v = (int)v * -1;
		c--;
	}

	p = buf + 64;
	*p = 0;
	do
	{
		c--;
		*--p = digits[v % base];
		v /= base;
	} while (v);

	if (negative)
		*--p = '-';

	if (fill != 0)
	{
		while (c > 0 && p > buf)
		{
			*--p = fill;
			c--;
		}
	}

	gfx_puts(p);
}

void gfx_printf(const char *fmt, ...)
{
	if (!gfx_con_init_done || gfx_con.mute)
		return;

	va_list ap;
	int fill, fcnt;

	va_start(ap, fmt);
	while (*fmt)
	{
		if (*fmt == '%')
		{
			fmt++;
			fill = 0;
			fcnt = 0;
			if ((*fmt >= '0' && *fmt <= '9') || *fmt == ' ')
			{
				fcnt = *fmt;
				fmt++;
				if (*fmt >= '0' && *fmt <= '9')
				{
					fill = fcnt;
					fcnt = *fmt - '0';
					fmt++;
				}
				else
				{
					fill = ' ';
					fcnt -= '0';
				}
			}
			switch(*fmt)
			{
			case 'c':
				gfx_putc(va_arg(ap, u32));
				break;
			case 's':
				gfx_puts(va_arg(ap, char *));
				break;
			case 'd':
				_gfx_putn(va_arg(ap, u32), 10, fill, fcnt);
				break;
			case 'p':
			case 'P':
			case 'x':
			case 'X':
				_gfx_putn(va_arg(ap, u32), 16, fill, fcnt);
				break;
			case 'k':
				gfx_con.fgcol = va_arg(ap, u32);
				break;
			case 'K':
				gfx_con.bgcol = va_arg(ap, u32);
				gfx_con.fillbg = 1;
				break;
			case '%':
				gfx_putc('%');
				break;
			case '\0':
				goto out;
			default:
				gfx_putc('%');
				gfx_putc(*fmt);
				break;
			}
		}
		else
			gfx_putc(*fmt);
		fmt++;
	}

	out:
	va_end(ap);
}

static void _gfx_cputs(u32 color, const char *s)
{
	gfx_con.fgcol = color;
	gfx_puts(s);
	gfx_putc('\n');
	gfx_con.fgcol = TXT_CLR_DEFAULT;
}

void gfx_wputs(const char *s) { _gfx_cputs(TXT_CLR_WARNING, s); }
void gfx_eputs(const char *s) { _gfx_cputs(TXT_CLR_ERROR,   s); }

void gfx_hexdump(u32 base, const void *buf, u32 len)
{
	if (!gfx_con_init_done || gfx_con.mute)
		return;

	u8 *buff = (u8 *)buf;

	u8 prevFontSize = gfx_con.fntsz;
	gfx_con.fntsz = 8;
	for(u32 i = 0; i < len; i++)
	{
		if(i % 0x10 == 0)
		{
			if(i != 0)
			{
				gfx_puts("| ");
				for(u32 j = 0; j < 0x10; j++)
				{
					u8 c = buff[i - 0x10 + j];
					if(c >= 32 && c <= 126)
						gfx_putc(c);
					else
						gfx_putc('.');
				}
				gfx_putc('\n');
			}
			gfx_printf("%08x: ", base + i);
		}
		gfx_printf("%02x ", buff[i]);
		if (i == len - 1)
		{
			int ln = len % 0x10 != 0;
			u32 k = 0x10 - 1;
			if (ln)
			{
				k = (len & 0xF) - 1;
				for (u32 j = 0; j < 0x10 - k; j++)
					gfx_puts("   ");
			}
			gfx_puts("| ");
			for(u32 j = 0; j < (ln ? k : k + 1); j++)
			{
				u8 c = buff[i - k + j];
				if(c >= 32 && c <= 126)
					gfx_putc(c);
				else
					gfx_putc('.');
			}
			gfx_putc('\n');
		}
	}
	gfx_putc('\n');
	gfx_con.fntsz = prevFontSize;
}

void gfx_set_pixel(u32 x, u32 y, u32 color)
{
	gfx_ctxt.fb[y + (gfx_ctxt.width - x) * gfx_ctxt.stride] = color;
}

void gfx_set_rect_pitch(u32 *fb, const u32 *buf, u32 stride, u32 pos_x, u32 pos_y, u32 pos_x2, u32 pos_y2)
{
	u32 *ptr = (u32 *)buf;
	u32 line_size = pos_x2 - pos_x + 1;
	//ptr = gfx_debug_rect(buf, pos_x, pos_y, pos_x2, pos_y2);
	for (u32 y = pos_y; y <= pos_y2; y++)
	{
		memcpy(&fb[pos_x + y * stride], ptr, line_size * sizeof(u32));
		ptr += line_size;
	}
}

void gfx_set_rect_land_pitch(u32 *fb, const u32 *buf, u32 stride, u32 pos_x, u32 pos_y, u32 pos_x2, u32 pos_y2)
{
	u32 *ptr = (u32 *)buf;

	u32 pixels_w = pos_x2 - pos_x + 1;

	if (!(pixels_w % 8))
	{
		for (u32 y = pos_y; y <= pos_y2; y++)
			for (u32 x = pos_x; x <= pos_x2; x += 8)
			{
				u32 *fbx = &fb[x * stride + y];

				fbx[0]          = *ptr++;
				fbx[stride]     = *ptr++;
				fbx[stride * 2] = *ptr++;
				fbx[stride * 3] = *ptr++;
				fbx[stride * 4] = *ptr++;
				fbx[stride * 5] = *ptr++;
				fbx[stride * 6] = *ptr++;
				fbx[stride * 7] = *ptr++;
			}
	}
	else
	{
		for (u32 y = pos_y; y < (pos_y2 + 1); y++)
			for (u32 x = pos_x; x < (pos_x2 + 1); x++)
				fb[x * stride + y] = *ptr++;
	}
}

void gfx_set_rect_land_block(u32 *fb, const u32 *buf, u32 pos_x, u32 pos_y, u32 pos_x2, u32 pos_y2)
{
	u32 *ptr = (u32 *)buf;
	u32 GOB_address = 0;
	u32 addr = 0;
	u32 x2 = 0;

	// Optimized
	u32 image_width_in_gobs = 655360; //1280
	for (u32 y = pos_y; y <= pos_y2; y++)
	{
		for (u32 x = pos_x; x <= pos_x2; x++)
		{
			GOB_address = (y >> 7) * image_width_in_gobs + ((x >> 4) << 13) + (((y % 128) >> 3) << 9);

			x2 = x << 2;
			addr = GOB_address
				+ (((x2 % 64) >> 5) << 8)
				+ (((y % 8) >> 1) << 6)
				+ (((x2 % 32) >> 4) << 5)
				+ ((y % 2) << 4) + (x2 % 16);

			*(u32 *)(fb + (addr >> 2)) = *ptr++;
		}
	}

	// Proper
	// u32 block_height = 16;
	// u32 image_width_in_gobs = (512 * block_height * 1280 * 4) / 64;
	// for (u32 y = pos_y; y <= pos_y2; y++)
	// {
	// 	for (int x = pos_x; x <= pos_x2; x++)
	// 	{
	// 		GOB_address = (y / (8 * block_height)) * image_width_in_gobs + ((x * 4 / 64) * 512 * block_height) + ((y % (8 * block_height) / 8) * 512);

	// 		x2 = x << 2;
	// 		addr = GOB_address
	// 			+ (((x2 % 64) >> 5) << 8)
	// 			+ (((y % 8) >> 1) << 6)
	// 			+ (((x2 % 32) >> 4) << 5)
	// 			+ ((y % 2) << 4) + (x2 % 16);

	// 		*(u32 *)(gfx_ctxt.fb + (addr >> 2)) = *ptr++;
	// 	}
	// }
}
