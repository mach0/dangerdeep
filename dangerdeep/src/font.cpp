/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2006  Thorsten Jordan, Luis Barrancos and others.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

// OpenGL based font rendering
// (C)+(W) by Thorsten Jordan. See LICENSE

#include "font.h"
#include "texture.h"
#include "system.h"
#include "oglext/OglExt.h"
#include <SDL.h>
#include <SDL_image.h>
#include <sstream>
#include <fstream>
using std::vector;
using std::string;
using std::ifstream;



font::character::~character()
{
	delete tex;
}

void font::print_text(int x, int y, const string& text, bool ignore_colors) const
{
	glActiveTexture(GL_TEXTURE1);	// fixme: disable units where they are used!
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	int xs = x;
	for (unsigned ti = 0; ti < text.length(); ti = character_right(text, ti)) {
		// read next unicode character
		unsigned c = read_character(text, ti);
		// if it is broken or illegal, skip it
		if (c == invalid_utf8_char) {
			continue;
		}
		if (c == ' ') {	// space
			x += blank_width;
		} else if (c == '\n') {	// return
			x = xs;
			y += height;
		} else if (c == '\t') {	// tab
			unsigned tw = height*4;
			x = int((x - xs + tw)/tw)*tw + xs;
		} else if (c == '$') { // color information
			unsigned nr[6];
			++ti;
			for (int i = 0; i < 6; ++i, ++ti) {
				if (ti >= text.length()) break;
				char c2 = text[ti];
				if (c2 >= '0' && c2 <= '9')
					nr[i] = c2 - '0';
				else if (c2 >= 'a' && c2 <= 'f')
					nr[i] = 10 + c2 - 'a';
				else
					nr[i] = 0;
			}
			--ti;	// compensate for(...++ti)
			if (!ignore_colors)
				glColor3ub(nr[0]*16+nr[1], nr[2]*16+nr[3], nr[4]*16+nr[5]);
		} else if (unsigned(c) >= first_char && unsigned(c) <= last_char) {
			unsigned t = unsigned(c) - first_char;
			// fixme: we could use display lists here
			float u1 = float(characters[t].width)/characters[t].tex->get_width();
			float v1 = float(characters[t].height)/characters[t].tex->get_height();
			//fixme: width in text is width+left, so advance x by that value
			//and draw at x+left. must be changed everywhere
			//int x2 = x + characters[t].left;
			int y2 = y + base_height - characters[t].top;
			glBindTexture(GL_TEXTURE_2D, characters[t].tex->get_opengl_name());
		        glBegin(GL_QUADS);
		        glTexCoord2f(0,0);
		        glVertex2i(x,y2); 
		        glTexCoord2f(0,v1);
	        	glVertex2i(x,y2+characters[t].height);
		        glTexCoord2f(u1,v1);
		        glVertex2i(x+characters[t].width,y2+characters[t].height);
		        glTexCoord2f(u1,0);
	        	glVertex2i(x+characters[t].width,y2);
		        glEnd();
			x += characters[t].width + spacing;
		} // else: just skip (unknown) character
	}
}

font::font(const string& basefilename, unsigned char_spacing)
{
	ifstream metricfile((basefilename + ".metric").c_str());
	metricfile >> base_height;
	metricfile >> first_char;
	metricfile >> last_char;
	characters.resize(last_char-first_char+1);
	for (unsigned i = first_char; i <= last_char; ++i) {
		if (!metricfile.good())
			throw error(string("error reading font metricfile for ")+basefilename);
		metricfile >> characters[i-first_char].width;
		metricfile >> characters[i-first_char].height;
		metricfile >> characters[i-first_char].left;
		metricfile >> characters[i-first_char].top;
	}
	
	spacing = char_spacing;
	unsigned codex = unsigned('x'), codeleftbr = unsigned('(');
	blank_width = (codex >= first_char && codex <= last_char) ? characters[codex-first_char].width : 8;
	//height = (codeleftbr >= first_char && codeleftbr <= last_char) ? characters[codeleftbr-first_char].height : base_height*3/2;
	height = base_height*3/2;
	base_height = base_height*7/6;	// tiny trick to use the space a bit better.
	
	// calculate offsets
	vector<unsigned> offsets(characters.size());
	for (unsigned i = 0, curoffset = 0; i < characters.size(); ++i) {
		offsets[i] = curoffset;
		curoffset += characters[i].width;
	}

	// load image
	SDL_Surface* fontimage = IMG_Load((basefilename + ".png").c_str());
	if (!fontimage)
		throw error(string("font: failed to open ")+basefilename);

	// process image data, create textures
	SDL_LockSurface(fontimage);
	if (fontimage->format->BytesPerPixel != 1)
		throw error(string("font: only grayscale images are supported! font ")+basefilename);

	for (unsigned i = 0; i < characters.size(); i++) {
		character& c = characters[i];
		unsigned w = next_p2(c.width);
		unsigned h = next_p2(c.height);
		vector<Uint8> tmpimage(w * h * 2);
		
		unsigned char* ptr = ((unsigned char*)(fontimage->pixels)) + offsets[i];
	
		for (unsigned y = 0; y < c.height; y++) {
			for (unsigned x = 0; x < c.width; ++x) {
				tmpimage[2*(y*w+x)] = 255;
				tmpimage[2*(y*w+x)+1] = *(ptr+x);
			}
			ptr += fontimage->pitch;
		}
		
		characters[i].tex = new texture(tmpimage, w, h, GL_LUMINANCE_ALPHA,
						texture::LINEAR, texture::CLAMP_TO_EDGE);
	}
	
	SDL_UnlockSurface(fontimage);
	SDL_FreeSurface(fontimage);
}



void font::print(int x, int y, const string& text, color col, bool with_shadow) const
{
	if (with_shadow) {
		glColor4ub(0,0,0,col.a);
		print_text(x+2, y+2, text, true);
	}
	col.set_gl_color();
	print_text(x, y, text);
}

void font::print_hc(int x, int y, const string& text, color col, bool with_shadow) const
{
	print(x-get_size(text).x/2, y, text, col, with_shadow);
}
	
void font::print_vc(int x, int y, const string& text, color col, bool with_shadow) const
{
	print(x, y-get_size(text).y/2, text, col, with_shadow);
}

void font::print_c(int x, int y, const string& text, color col, bool with_shadow) const
{
	vector2i wh = get_size(text);
	print(x-wh.x/2, y-wh.y/2, text, col, with_shadow);
}

void font::print_wrapped(int x, int y, unsigned w, unsigned lineheight, const string& text, color col, bool with_shadow) const
{
	// loop over spaces
	unsigned currwidth = 0;
	unsigned textptr = 0, oldtextptr = 0;
	unsigned textlen = text.length();
	while (true) {
		// remove spaces, treat returns
		while (textptr < textlen) {	// remove spaces at the beginning
			char c = text[textptr];
			if (c == '\n') {
				y += (lineheight == 0) ? get_height() : lineheight;
				currwidth = 0;
				++textptr;
			} else if (c == ' ') {
				++textptr;
			} else if (c == '\t') {
				++textptr;
			} else {
				break;
			}
		}
		oldtextptr = textptr;
		// collect characters for current word
		unsigned charw = 0;
		bool breaktext = false;
		while ((text[textptr] != '\n' && text[textptr] != ' ' && text[textptr] != '\t') && textptr < textlen) {
			unsigned c = read_character(text, textptr);
			charw += get_char_width(c);
			if (currwidth == 0 && charw >= w) {	// word is longer than line
				breaktext = true;
				break;
			}
			charw += spacing;
			textptr = character_right(text, textptr);
		}
		if (textlen > 0 && textptr == 0) {	// space is not enough to wrap first word. so disable wrapping
			w = 0xffffffff;
			continue;
		}
		if (breaktext) {
			print(x, y, text.substr(oldtextptr, textptr - oldtextptr), col, with_shadow);
			y += (lineheight == 0) ? get_height() : lineheight;
		} else {
			unsigned tw = get_size(text.substr(oldtextptr, textptr - oldtextptr)).x;
			if (currwidth + tw >= w) {
				y += (lineheight == 0) ? get_height() : lineheight;
				currwidth = 0;
			}
			print(x+currwidth, y, text.substr(oldtextptr, textptr - oldtextptr), col, with_shadow);
			currwidth += tw + blank_width;
		}
		if (textptr == textlen)
			break;
	}
}

vector2i font::get_size(const string& text) const
{
	unsigned x = 0, y = height;
	unsigned xmax = 0;
	for (unsigned ti = 0; ti < text.length(); ti = character_right(text, ti)) {
		// read next unicode character
		unsigned c = read_character(text, ti);
		// if it is broken or illegal, skip it
		if (c == invalid_utf8_char) {
			continue;
		}
		if (c == ' ') {	// space
			x += blank_width;
		} else if (c == '\n') {	// return
			x = 0;
			y += height;
		} else if (c == '\t') {	// tab
			unsigned tw = height*4;
			x = int((x + tw)/tw)*tw;
		} else if (c == '$') { // color information
			unsigned nr[6];
			++ti;
			for (int i = 0; i < 6; ++i, ++ti) {
				if (ti >= text.length()) break;
				char c2 = text[ti];
				if (c2 >= '0' && c2 <= '9')
					nr[i] = c2 - '0';
				else if (c2 >= 'a' && c2 <= 'f')
					nr[i] = 10 + c2 - 'a';
				else
					nr[i] = 0;
			}
			--ti;	// compensate for(...++ti)
		} else if (unsigned(c) >= first_char && unsigned(c) <= last_char) {
			unsigned t = unsigned(c) - first_char;
			x += characters[t].width + spacing;
		} // else: ignore (unknown) character
		if (x > xmax) xmax = x;
	}
	if (x == 0) y -= height;
	return vector2i(xmax, y);
}



unsigned font::get_char_width(unsigned c) const
{
	if (c == ' ') {
		return blank_width;
	} else if (c >= first_char && c <= last_char) {
		return characters[unsigned(c) - first_char].width;
	} else {
		return 0;
	}
}



unsigned font::character_left(const std::string& text, unsigned cp)
{
	if (cp > 0) {
		// move one left
		--cp;
		// if on multibyte character, but not first, and we can move left, move further
		while (cp > 0 && is_byte_of_multibyte_char(text[cp]) && !is_first_byte_of_multibyte_char(text[cp]))
			--cp;
	}
	return cp;
}



unsigned font::character_right(const std::string& text, unsigned cp)
{
	const unsigned l = text.size();
	if (cp < l) {
		// check if we are on multibyte char
		if (is_byte_of_multibyte_char(text[cp])) {
			++cp;
			while (cp < l) {
				if (is_byte_of_multibyte_char(text[cp])) {
					if (is_first_byte_of_multibyte_char(text[cp])) {
						break;
					}
				} else {
					break;
				}
				++cp;
			}
		} else {
			++cp;
		}
	}
	return cp;
}



unsigned font::read_character(const std::string& text, unsigned cp)
{
	// Unicode (UTF-8) decoder:
	// In UTF-8 all characters from 0x00-0x7F are encoded as one byte.
	// Characters from 0x80-0x7FF are encoded as two bytes:
	// 0xC0 | (upper 5 bits of c), 0x80 | (lower 6 bits of c)
	// This is sufficient to encode 11bits of characters.
	// Characters from 0x0800-0xffff are encoded as three bytes:
	// 0xE0 | (upper 4 bits of c), 0x80 | (middle 6 bits of c), 0x80 | (lower 6 bits of c)
	// Characters from 0x010000-0x10FFFF are encoded as four bytes:
	// 0xF0 | (upper 3 bits), (0x80 | (further 6 bits)) * 3
	// We only use the lower 256 characters of Unicode, which map to
	// ISO-8859-1.
	// We could either copy the character value directly from the source
	// or translate it from UTF-8.
	unsigned char c = text[cp];
	if (c & 0x80) {
		if ((c & 0xE0) == 0xC0) {
			// UTF-8 2 byte extension
			// fetch next byte
			if (cp + 1 < text.length()) {
				unsigned char c2 = text[cp + 1];
				// combine bytes to 8bit character
				return (unsigned(c & 0x1F) << 6) | (c2 & 0x3F);
			} else {
				// invalid UTF-8 extension at end of string...
				return invalid_utf8_char;
			}
		// fixme: later extend to three byte UTF-8 characters here...
		} else {
			// invalid character, skip
			return invalid_utf8_char;
		}
	} // else: normal character
	return c;
}
