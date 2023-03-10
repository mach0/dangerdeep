/*
Danger from the Deep - Open source submarine simulation
Copyright (C) 2003-2020  Thorsten Jordan, Luis Barrancos and others.

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

// SDL/OpenGL based textures
// (C)+(W) by Thorsten Jordan. See LICENSE

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "log.h"
#include "oglext/OglExt.h"
#include "primitives.h"
#include "system_interface.h"
#include "texture.h"
#include "vector3.h"

#include <SDL.h>
#include <SDL_image.h>
#include <fstream>
#include <glu.h>
#include <iostream>
#include <sstream>
#include <vector>
using namespace std;

#undef MEMMEASURE

#ifdef MEMMEASURE
unsigned texture::mem_used    = 0;
unsigned texture::mem_alloced = 0;
unsigned texture::mem_freed   = 0;
#endif

int texture::size_non_power_2           = -1;
bool texture::use_compressed_textures   = false;
bool texture::use_anisotropic_filtering = false;
float texture::anisotropic_level        = 0.0f;

auto texture::size_non_power_two() -> bool
{
    return true;
}

// ------------------------------- GL mode tables -------------------
static GLuint mapmodes[texture::NR_OF_MAPPING_MODES] = {
    GL_NEAREST,
    GL_LINEAR,
    GL_NEAREST_MIPMAP_NEAREST,
    GL_NEAREST_MIPMAP_LINEAR,
    GL_LINEAR_MIPMAP_NEAREST,
    GL_LINEAR_MIPMAP_LINEAR};

static bool do_mipmapping[texture::NR_OF_MAPPING_MODES] =
    {false, false, true, true, true, true};

static GLuint magfilter[texture::NR_OF_MAPPING_MODES] =
    {GL_NEAREST, GL_LINEAR, GL_NEAREST, GL_NEAREST, GL_LINEAR, GL_LINEAR};

static GLuint clampmodes[texture::NR_OF_CLAMPING_MODES] = {
    GL_REPEAT,
    GL_CLAMP_TO_EDGE};
// --------------------------------------------------

sdl_image::sdl_image(const std::string& filename) : img(nullptr)
{
    // get extension
    string::size_type st = filename.rfind('.');
    string extension     = filename.substr(st);

    if (extension != ".jpg|png")
    {
        // standard texture, just one file
        img = IMG_Load(filename.c_str());
        if (!img)
        {
            THROW(file_read_error, filename);
        }
    }
    else
    {
        // special texture, using jpg for RGB and png/grey for A.
        string fnrgb = filename.substr(0, st) + ".jpg";
        string fna   = filename.substr(0, st) + ".png";
        // "recursive" use of constructor. looks wild, but is valid.
        sdl_image teximagergb(fnrgb);
        sdl_image teximagea(fna);

        // combine surfaces to one
        if (teximagergb->w != teximagea->w || teximagergb->h != teximagea->h)
        {
            THROW(
                texture::texerror,
                filename,
                "jpg/png load: widths/heights don't match");
        }

        if (teximagergb->format->BytesPerPixel != 3
            || (teximagergb->format->Amask != 0))
        {
            THROW(texture::texerror, fnrgb, ".jpg: no 3 byte/pixel RGB image!");
        }

        uint32_t color_key = 0;
        bool usealpha =
            SDL_GetColorKey(teximagea.get_SDL_Surface(), &color_key) == 0;
        if (teximagea->format->BytesPerPixel != 1
            || teximagea->format->palette == nullptr
            || teximagea->format->palette->ncolors != 256 || usealpha)
        {
            THROW(
                texture::texerror,
                fna,
                ".png: no 8bit greyscale non-alpha-channel image!");
        }

        uint32_t rmask, gmask, bmask, amask;

        /* SDL interprets each pixel as a 32-bit number, so our masks must
           depend on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
        rmask = 0xff000000;
        gmask = 0x00ff0000;
        bmask = 0x0000ff00;
        amask = 0x000000ff;
#else
        rmask = 0x000000ff;
        gmask = 0x0000ff00;
        bmask = 0x00ff0000;
        amask = 0xff000000;
#endif

        SDL_Surface* result = SDL_CreateRGBSurface(
            SDL_SWSURFACE,
            teximagergb->w,
            teximagergb->h,
            32,
            rmask,
            gmask,
            bmask,
            amask);
        if (!result)
        {
            THROW(file_read_error, filename);
        }

        try
        {
            // copy pixel data
            teximagergb.lock();
            teximagea.lock();
            SDL_LockSurface(result);

            // fixme: when reading pixels out of sdl surfaces,
            // we need to take care of the pixel format...
            unsigned char* ptr       = (static_cast<unsigned char*>(result->pixels));
            unsigned char* offsetrgb = (static_cast<unsigned char*>(teximagergb->pixels));
            unsigned char* offseta   = (static_cast<unsigned char*>(teximagea->pixels));
            // 2006-12-01 doc1972 images with negative width and height doesn?
            // exist, so we cast to unsigned
            for (unsigned y = 0; y < static_cast<unsigned int>(teximagergb->h); ++y)
            {
                for (unsigned x = 0; x < static_cast<unsigned int>(teximagergb->w); ++x)
                {
                    ptr[4 * x]     = offsetrgb[3 * x];
                    ptr[4 * x + 1] = offsetrgb[3 * x + 1];
                    ptr[4 * x + 2] = offsetrgb[3 * x + 2];
                    ptr[4 * x + 3] = offseta[x];
                }
                offsetrgb += teximagergb->pitch;
                offseta += teximagea->pitch;
                ptr += result->pitch;
            }

            teximagergb.unlock();
            teximagea.unlock();
            SDL_UnlockSurface(result);
        }
        catch (...)
        {
            if (result)
            {
                SDL_FreeSurface(result);
            }
            throw;
        }

        img = result;
    }
}

sdl_image::~sdl_image()
{
    SDL_FreeSurface(img);
}

void sdl_image::lock()
{
    SDL_LockSurface(img);
}

void sdl_image::unlock()
{
    SDL_UnlockSurface(img);
}

auto sdl_image::get_plain_data(
    unsigned& w,
    unsigned& h,
    unsigned& byte_per_pixel) -> std::vector<uint8_t>
{
    byte_per_pixel = img->format->BytesPerPixel;
    w              = img->w;
    h              = img->h;
    std::vector<uint8_t> tmp(w * h * byte_per_pixel);
    lock();
    for (unsigned y = 0; y < h; ++y)
    {
        memcpy(
            &tmp[y * w * byte_per_pixel],
            static_cast<uint8_t*>(img->pixels) + y * img->pitch,
            w * byte_per_pixel);
    }
    unlock();
    return tmp;
}

auto sdl_image::get_width() const -> unsigned
{
    return img->w;
}
auto sdl_image::get_height() const -> unsigned
{
    return img->h;
}

// --------------------------------------------------

void texture::sdl_init(
    SDL_Surface* teximage,
    unsigned sx,
    unsigned sy,
    unsigned sw,
    unsigned sh,
    bool makenormalmap,
    float detailh,
    bool rgb2grey)
{
    // compute texture width and height
    unsigned tw = sw, th = sh;
    if (!size_non_power_two())
    {
        tw = 1;
        th = 1;
        while (tw < sw)
        {
            tw *= 2;
        }
        while (th < sh)
        {
            th *= 2;
        }
    }
    width     = sw;
    height    = sh;
    gl_width  = tw;
    gl_height = th;

    SDL_LockSurface(teximage);

    const SDL_PixelFormat& fmt = *(teximage->format);
    unsigned bpp               = fmt.BytesPerPixel;

    /*
        cout << "texture: " << texfilename
             << " palette: " << teximage->format->palette
             << " bpp " << unsigned(teximage->format->BitsPerPixel)
             << " bytepp " << unsigned(teximage->format->BytesPerPixel)
             << " Rmask " << teximage->format->Rmask
             << " Gmask " << teximage->format->Gmask
             << " Bmask " << teximage->format->Bmask
             << " Amask " << teximage->format->Amask
             << "\n";
    */

    vector<uint8_t> data;
    if (fmt.palette != nullptr)
    {
        // old color table code, does not work
        // glEnable(GL_COLOR_TABLE);
        if (bpp != 1)
        {
            THROW(texerror, get_name(), "only 8bit palette files supported");
        }
        int ncol = fmt.palette->ncolors;
        if (ncol > 256)
        {
            THROW(texerror, get_name(), "max. 256 colors in palette supported");
        }
        uint32_t color_key = 0;
        bool usealpha      = SDL_GetColorKey(teximage, &color_key) == 0;

        // check for greyscale images (GL_LUMINANCE), fixme: add also
        // LUMINANCE_ALPHA!
        bool lumi = false;
        if (ncol == 256 && !usealpha)
        {
            unsigned i = 0;
            for (; i < 256; ++i)
            {
                if (unsigned(fmt.palette->colors[i].r) != i)
                {
                    break;
                }
                if (unsigned(fmt.palette->colors[i].g) != i)
                {
                    break;
                }
                if (unsigned(fmt.palette->colors[i].b) != i)
                {
                    break;
                }
            }
            if (i == 256)
            {
                lumi = true;
            }
        }

        if (lumi)
        {
            // grey value images
            format = GL_LUMINANCE;
            bpp    = 1;

            data.resize(tw * th * bpp);
            unsigned char* ptr    = &data[0];
            unsigned char* offset = (static_cast<unsigned char*>(teximage->pixels))
                                    + sy * teximage->pitch + sx;
            for (unsigned y = 0; y < sh; y++)
            {
                memcpy(ptr, offset, sw /* * bpp */);
                offset += teximage->pitch;
                ptr += tw * bpp;
            }
        }
        else
        {
            // color images
            format = usealpha ? GL_RGBA : GL_RGB;
            bpp    = usealpha ? 4 : 3;

            // old color table code, does not work
            // glColorTable(GL_TEXTURE_2D, internalformat, 256, GL_RGBA,
            // GL_UNSIGNED_BYTE, &(palette[0])); internalformat =
            // GL_COLOR_INDEX8_EXT; externalformat = GL_COLOR_INDEX;
            data.resize(tw * th * bpp);
            unsigned char* ptr    = &data[0];
            unsigned char* offset = (static_cast<unsigned char*>(teximage->pixels))
                                    + sy * teximage->pitch + sx;
            for (unsigned y = 0; y < sh; y++)
            {
                unsigned char* ptr2 = ptr;
                for (unsigned x = 0; x < sw; ++x)
                {
                    uint8_t pixindex          = *(offset + x);
                    const SDL_Color& pixcolor = fmt.palette->colors[pixindex];
                    *ptr2++                   = pixcolor.r;
                    *ptr2++                   = pixcolor.g;
                    *ptr2++                   = pixcolor.b;
                    if (usealpha)
                    {
                        *ptr2++ =
                            (pixindex == (color_key & 0xff)) ? 0x00 : 0xff;
                    }
                }
                // old color table code, does not work
                // memcpy(ptr, offset, sw);
                offset += teximage->pitch;
                ptr += tw * bpp;
            }
        }
    }
    else
    {
        bool usealpha = fmt.Amask != 0;
        if (rgb2grey)
        {
            if (usealpha)
            {
                format = GL_LUMINANCE_ALPHA;
                bpp    = 2;
            }
            else
            {
                format = GL_LUMINANCE;
                bpp    = 1;
            }
        }
        else
        {
            if (usealpha)
            {
                format = GL_RGBA;
                bpp    = 4;
            }
            else
            {
                format = GL_RGB;
                bpp    = 3;
            }
        }
        data.resize(tw * th * bpp);
        unsigned char* ptr    = &data[0];
        unsigned char* offset = (static_cast<unsigned char*>(teximage->pixels))
                                + sy * teximage->pitch + sx * bpp;
        if (rgb2grey)
        {
            for (unsigned y = 0; y < sh; y++)
            {
                for (unsigned x = 0; x < sw; ++x)
                {
                    ptr[x * bpp] =
                        offset[x * (bpp + 2) + 1]; // take green value, it
                }
                // doesn't matter
                if (bpp == 2)
                {
                    // with alpha
                    for (unsigned x = 0; x < sw; ++x)
                    {
                        ptr[x * 2 + 1] = offset[x * 4 + 3];
                    }
                }
                offset += teximage->pitch;
                ptr += tw * bpp;
            }
        }
        else
        {
            for (unsigned y = 0; y < sh; y++)
            {
#if 0
				// new code, that uses the RGB-masks of SDL to load/transform
				// colors.
				if (bpp == 3) {
					uint8_t* linedst = (uint8_t*)ptr;
					uint8_t* linesrc = (uint8_t*)offset;
					for (unsigned x = 0; x < sw; ++x) {
						// be careful! with bpp=3 this could lead to
						// an off by one segfault error, if pitch is
						// not a multiple of four...we just hope the best.
						// could be done quicker with mmx, but this performance
						// is not that critical here
						uint32_t pv = *(uint32_t*)linesrc; // fixme: is that right for Big-Endian machines? SDL Docu suggests yes...
						linedst[0] = uint8_t(((pv & fmt.Rmask) >> fmt.Rshift) << fmt.Rloss);
						linedst[1] = uint8_t(((pv & fmt.Gmask) >> fmt.Gshift) << fmt.Gloss);
						linedst[2] = uint8_t(((pv & fmt.Bmask) >> fmt.Bshift) << fmt.Bloss);
						linesrc += bpp;
						linedst += bpp;
					}
				} else {
					// bpp = 4 here
					uint32_t* linedst = (uint32_t*)ptr;
					uint32_t* linesrc = (uint32_t*)offset;
					for (unsigned x = 0; x < sw; ++x) {
						uint32_t pv = linesrc[x];
						uint32_t r = (((pv & fmt.Rmask) >> fmt.Rshift) << fmt.Rloss);
						uint32_t g = (((pv & fmt.Gmask) >> fmt.Gshift) << fmt.Gloss);
						uint32_t b = (((pv & fmt.Bmask) >> fmt.Bshift) << fmt.Bloss);
						uint32_t a = (((pv & fmt.Bmask) >> fmt.Bshift) << fmt.Bloss);
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
						pv = a | (b << 8) | (g << 16) | (r << 24);
#else
						pv = r | (g << 8) | (b << 16) | (a << 24);
#endif
						linedst[x] = pv;
					}
				}
#endif
                // old code, that assumes bytes come in R,G,B order:
                memcpy(ptr, offset, sw * bpp);
                offset += teximage->pitch;
                ptr += tw * bpp;
            }
        }
    }
    SDL_UnlockSurface(teximage);
    init(data, makenormalmap, detailh);
}

void texture::init(
    const vector<uint8_t>& data,
    bool makenormalmap,
    float detailh)
{
    // error checks.
    if (mapping < 0 || mapping >= NR_OF_MAPPING_MODES)
    {
        THROW(texerror, get_name(), "illegal mapping mode!");
    }
    if (clamping < 0 || clamping >= NR_OF_CLAMPING_MODES)
    {
        THROW(texerror, get_name(), "illegal clamping mode!");
    }

    unsigned ms = get_max_size();
    if (width > ms || height > ms)
    {
        THROW(
            texerror,
            texfilename,
            "texture values too large, not supported by card");
    }

    glGenTextures(1, &opengl_name);
    glBindTexture(dimension, opengl_name);

#ifdef MEMMEASURE
    unsigned add_mem_used = 0;
#endif

    if (makenormalmap && format == GL_LUMINANCE)
    {
        if (dimension != GL_TEXTURE_2D)
        {
            THROW(
                texerror, get_name(), "normals only supported for 2D textures");
        }
        // make own mipmap building for normal maps here...
        // give increasing levels with decreasing w/h down to 1x1
        // e.g. 64x16 -> 32x8, 16x4, 8x2, 4x1, 2x1, 1x1
        format = GL_RGB;
        vector<uint8_t> nmpix =
            make_normals(data, gl_width, gl_height, detailh);
        int internalformat = format;

        if (use_compressed_textures)
        {
            format = GL_COMPRESSED_LUMINANCE_ARB;
        }

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            internalformat,
            gl_width,
            gl_height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            &nmpix[0]);

#ifdef MEMMEASURE
        add_mem_used = gl_width * gl_height * get_bpp();
#endif

        if (do_mipmapping[mapping])
        {
            if (dimension != GL_TEXTURE_2D)
            {
                THROW(
                    texerror,
                    get_name(),
                    "mip mapping only supported for 2D textures");
            }
#if 1
            // fixme: doesn't work with textures that don't have power of two
            // size...
            gluBuild2DMipmaps(
                GL_TEXTURE_2D,
                format,
                gl_width,
                gl_height,
                format,
                GL_UNSIGNED_BYTE,
                &nmpix[0]);

#else
            // buggy version. gives white textures. maybe some mipmap levels
            // are missing so that gl complains by make white textures?
            // fixme: test it again after the mapping/clamping bugfix!

            // fixme: if we let GLU do the mipmap calculation, the result is
            // wrong. A filtered version of the normals is not the same as a
            // normal map of the filtered height field! E.g. scaling down the
            // map to 1x1 pixel gives a medium height of 128, that is a flat
            // plane with a normal of (0,0,1) But filtering down the normals to
            // one pixel could give RGB=0.5 -> normal of (0,0,0) (rare...)!
            // fixme: mipmapping for textures with non-power-of-two resolution
            // is untested!
            vector<uint8_t> curlvl;
            const vector<uint8_t>* gdat = &data;
            for (unsigned level = 1, w = gl_width / 2, h = gl_height / 2;
                 w > 0 && h > 0;
                 w /= 2, h /= 2)
            {
                cout << "level " << level << " w " << w << " h " << h << "\n";
                curlvl = scale_half(*gdat, w, h, 1);
                gdat   = &curlvl;
                // fixme: must detailh also get halfed here? yes...
                vector<uint8_t> nmpix = make_normals(*gdat, w, h, detailh);
                int internalformat    = GL_RGB;

                if (use_compressed_textures)
                    internalformat = GL_COMPRESSED_RGB_ARB;

                glTexImage2D(
                    GL_TEXTURE_2D,
                    level,
                    internalformat,
                    w,
                    h,
                    0,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    &nmpix[0]);
                w /= 2;
                h /= 2;
            }
#endif
        }
    }
    else if (makenormalmap && format == GL_LUMINANCE_ALPHA)
    {
        format = GL_RGBA;
        vector<uint8_t> nmpix =
            make_normals_with_alpha(data, gl_width, gl_height, detailh);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            format,
            gl_width,
            gl_height,
            0,
            format,
            GL_UNSIGNED_BYTE,
            &nmpix[0]);
#ifdef MEMMEASURE
        add_mem_used = gl_width * gl_height * get_bpp();
#endif
        if (do_mipmapping[mapping])
        {
            // fixme: doesn't work with textures that don't have power of two
            // size...
            gluBuild2DMipmaps(
                GL_TEXTURE_2D,
                format,
                gl_width,
                gl_height,
                format,
                GL_UNSIGNED_BYTE,
                &nmpix[0]);
        }
    }
    else
    {
        // make gl texture
        int internalformat = format;

        if (use_compressed_textures)
        {
            switch (format)
            {
                case GL_RGB:
                    internalformat = GL_COMPRESSED_RGB_ARB;
                    break;
                case GL_RGBA:
                    internalformat = GL_COMPRESSED_RGBA_ARB;
                    break;
                case GL_LUMINANCE:
                    internalformat = GL_COMPRESSED_LUMINANCE_ARB;
                    break;
                case GL_LUMINANCE_ALPHA:
                    internalformat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB;
                    break;
            }
        }
        switch (dimension)
        {
            case GL_TEXTURE_2D:
            {
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    internalformat,
                    gl_width,
                    gl_height,
                    0,
                    format,
                    GL_UNSIGNED_BYTE,
                    &data[0]);
            }
            break;
            case GL_TEXTURE_1D:
            {
                glTexImage1D(
                    GL_TEXTURE_1D,
                    0,
                    internalformat,
                    max(gl_width, gl_height),
                    0,
                    format,
                    GL_UNSIGNED_BYTE,
                    &data[0]);
            }
            break;
        }

#ifdef MEMMEASURE
        add_mem_used = gl_width * gl_height * get_bpp();
#endif
        if (do_mipmapping[mapping])
        {
            // fixme: does this command set the base level, too?
            // i.e. are the two gl commands redundant?
            // fixme: doesn't work with textures that don't have power of two
            // size...
            gluBuild2DMipmaps(
                GL_TEXTURE_2D,
                format,
                gl_width,
                gl_height,
                format,
                GL_UNSIGNED_BYTE,
                &data[0]);
        }
    }

#ifdef MEMMEASURE
    if (do_mipmapping[mapping])
        add_mem_used = (4 * add_mem_used) / 3;
    mem_used += add_mem_used;
    log_debug(
        "Allocated " << add_mem_used << " bytes of video memory for texture '"
                     << texfilename << "', total video mem use "
                     << mem_used / 1024 << " kb");
    mem_alloced += add_mem_used;
    log_debug("Video mem usage " << mem_alloced << " vs " << mem_freed);
#endif

    glTexParameteri(dimension, GL_TEXTURE_MIN_FILTER, mapmodes[mapping]);
    glTexParameteri(dimension, GL_TEXTURE_MAG_FILTER, magfilter[mapping]);
    glTexParameteri(dimension, GL_TEXTURE_WRAP_S, clampmodes[clamping]);
    glTexParameteri(dimension, GL_TEXTURE_WRAP_T, clampmodes[clamping]);

    // enable anisotropic filtering if choosen
    if (use_anisotropic_filtering)
    {
        glTexParameterf(
            dimension, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_level);
    }
}

#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                         \
    ((int32_t)(int8_t)(ch0) | ((int32_t)(int8_t)(ch1) << 8)                    \
     | ((int32_t)(int8_t)(ch2) << 16) | ((int32_t)(int8_t)(ch3) << 24))
void texture::load_dds(const std::string& filename, dds_data& target)
{
    DDSHEAD header;
    std::ifstream file;
    int factor;
    int bufferSize;

    // Open the file
    file.open(filename.c_str());

    if (!file.good())
    {
        THROW(error, "couldn't find, or failed to load " + filename);
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(header));

    if (std::string(reinterpret_cast<char*>(header.Signature), 4) != "DDS ")
    {
        THROW(error, "not a valid .dds file: " + filename);
    }

    //
    // This .dds loader supports the loading of compressed formats DXT1, DXT3
    // and DXT5.
    //
    target.components = 4;

    switch (SDL_SwapLE32(header.FourCC))
    {
        case MAKEFOURCC('D', 'X', 'T', '1'):
            // DXT1's compression ratio is 8:1
            target.format     = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
            target.components = 3;
            factor            = 2;
            break;

        case MAKEFOURCC('D', 'X', 'T', '3'):
            // DXT3's compression ratio is 4:1
            target.format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            factor        = 4;
            break;

        case MAKEFOURCC('D', 'X', 'T', '5'):
            // DXT5's compression ratio is 4:1
            target.format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            factor        = 4;
            break;

        default:
            THROW(error, "no supported compression type on file: " + filename);
    }

    // How big will the buffer need to be to load all of the pixel data
    // including mip-maps?

    if (SDL_SwapLE32(header.LinearSize) == 0)
    {
        THROW(error, "linear size in dds file is 0: " + filename);
    }

    if (header.MipMapCount > 1)
    {
        bufferSize = SDL_SwapLE32(header.LinearSize) * factor;
    }
    else
    {
        bufferSize = SDL_SwapLE32(header.LinearSize);
    }

    target.pixels.resize(bufferSize);

    file.read(reinterpret_cast<char*>(&target.pixels[0]), bufferSize);

    // Close the file
    file.close();

    target.width      = SDL_SwapLE32(header.Width);
    target.height     = SDL_SwapLE32(header.Height);
    target.numMipMaps = SDL_SwapLE32(header.MipMapCount);
}
#undef MAKEFOURCC

auto texture::scale_half(
    const vector<uint8_t>& src,
    unsigned w,
    unsigned h,
    unsigned bpp) -> vector<uint8_t>
{
    if (!size_non_power_two())
    {
        if (w < 1 || (w & (w - 1)) != 0)
        {
            THROW(
                texerror, "[scale_half]", "texture width is no power of two!");
        }
        if (h < 1 || (h & (h - 1)) != 0)
        {
            THROW(
                texerror, "[scale_half]", "texture height is no power of two!");
        }
    }

    vector<uint8_t> dst(w * h * bpp / 4);
    unsigned ptr = 0;
    for (unsigned y = 0; y < h; y += 2)
    {
        for (unsigned x = 0; x < w; x += 2)
        {
            for (unsigned b = 0; b < bpp; ++b)
            {
                dst[ptr++] = uint8_t(
                    (unsigned(src[(y * w + x) * bpp + b])
                     + unsigned(src[(y * w + x + 1) * bpp + b])
                     + unsigned(src[((y + 1) * w + x) * bpp + b])
                     + unsigned(src[((y + 1) * w + x + 1) * bpp + b]))
                    / 4);
            }
        }
    }
    return dst;
}

auto texture::make_normals(
    const vector<uint8_t>& src,
    unsigned w,
    unsigned h,
    float detailh) -> vector<uint8_t>
{
    // src size must be w*h
    vector<uint8_t> dst(3 * w * h);
    // Note! zh must be multiplied with 2*sample_distance!
    // sample_distance is real distance between texels, we assume 1 for it...
    // This depends on the size of the face the normal map is mapped onto.
    // but all other code is written to match 255/detailh, especially
    // bump scaling in model.cpp, so don't change this!
    float zh     = /* 2.0f* */ 255.0f / detailh;
    unsigned ptr = 0;
    for (unsigned yy = 0; yy < h; ++yy)
    {
        unsigned y1 = (yy + h - 1) & (h - 1);
        unsigned y2 = (yy + 1) & (h - 1);
        for (unsigned xx = 0; xx < w; ++xx)
        {
            unsigned x1  = (xx + w - 1) & (w - 1);
            unsigned x2  = (xx + 1) & (w - 1);
            float hr     = src[yy * w + x2];
            float hu     = src[y1 * w + xx];
            float hl     = src[yy * w + x1];
            float hd     = src[y2 * w + xx];
            vector3f nm  = vector3f(hl - hr, hd - hu, zh).normal();
            dst[ptr + 0] = uint8_t(nm.x * 127 + 128);
            dst[ptr + 1] = uint8_t(nm.y * 127 + 128);
            dst[ptr + 2] = uint8_t(nm.z * 127 + 128);
            ptr += 3;
        }
    }
    return dst;
}

auto texture::make_normals_with_alpha(
    const vector<uint8_t>& src,
    unsigned w,
    unsigned h,
    float detailh) -> vector<uint8_t>
{
    // src size must be w*h
    vector<uint8_t> dst(4 * w * h);
    // Note! zh must be multiplied with 2*sample_distance!
    // sample_distance is real distance between texels, we assume 1 for it...
    // This depends on the size of the face the normal map is mapped onto.
    // but all other code is written to match 255/detailh, especially
    // bump scaling in model.cpp, so don't change this!
    float zh     = /* 2.0f* */ 255.0f / detailh;
    unsigned ptr = 0;
    for (unsigned yy = 0; yy < h; ++yy)
    {
        unsigned y1 = (yy + h - 1) & (h - 1);
        unsigned y2 = (yy + 1) & (h - 1);
        for (unsigned xx = 0; xx < w; ++xx)
        {
            unsigned x1  = (xx + w - 1) & (w - 1);
            unsigned x2  = (xx + 1) & (w - 1);
            float hr     = src[2 * (yy * w + x2) + 0];
            float hu     = src[2 * (y1 * w + xx) + 0];
            float hl     = src[2 * (yy * w + x1) + 0];
            float hd     = src[2 * (y2 * w + xx) + 0];
            vector3f nm  = vector3f(hl - hr, hd - hu, zh).normal();
            dst[ptr + 0] = uint8_t(nm.x * 127 + 128);
            dst[ptr + 1] = uint8_t(nm.y * 127 + 128);
            dst[ptr + 2] = uint8_t(nm.z * 127 + 128);
            dst[ptr + 3] = src[2 * (yy * w + xx) + 1];
            ptr += 4;
        }
    }
    return dst;
}

texture::texture(
    const string& filename,
    mapping_mode mapping_,
    clamping_mode clamp,
    bool makenormalmap,
    float detailh,
    bool rgb2grey,
    GLenum _dimension)
{
    dimension   = _dimension;
    mapping     = mapping_;
    clamping    = clamp;
    texfilename = filename;

    sdl_image teximage(filename);
    sdl_init(
        teximage.get_SDL_Surface(),
        0,
        0,
        teximage->w,
        teximage->h,
        makenormalmap,
        detailh,
        rgb2grey);
}

texture::texture(
    SDL_Surface* teximage,
    unsigned sx,
    unsigned sy,
    unsigned sw,
    unsigned sh,
    mapping_mode mapping_,
    clamping_mode clamp,
    bool makenormalmap,
    float detailh,
    bool rgb2grey,
    GLenum _dimension)
{
    dimension = _dimension;
    mapping   = mapping_;
    clamping  = clamp;
    sdl_init(teximage, sx, sy, sw, sh, makenormalmap, detailh, rgb2grey);
}

texture::texture(
    const sdl_image& teximage,
    unsigned sx,
    unsigned sy,
    unsigned sw,
    unsigned sh,
    mapping_mode mapping_,
    clamping_mode clamp,
    bool makenormalmap,
    float detailh,
    bool rgb2grey,
    GLenum _dimension)
{
    dimension = _dimension;
    mapping   = mapping_;
    clamping  = clamp;
    sdl_init(
        teximage.get_SDL_Surface(),
        sx,
        sy,
        sw,
        sh,
        makenormalmap,
        detailh,
        rgb2grey);
}

texture::texture(
    const vector<uint8_t>& pixels,
    unsigned w,
    unsigned h,
    int format_,
    mapping_mode mapping_,
    clamping_mode clamp,
    bool makenormalmap,
    float detailh,
    GLenum _dimension)
{
    dimension = _dimension;
    mapping   = mapping_;
    clamping  = clamp;

    if (!size_non_power_two())
    {
        if (w < 1 || (w & (w - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture width is no power of two!");
        }
        if (h < 1 || (h & (h - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture height is no power of two!");
        }
    }

    width = gl_width = w;
    height = gl_height = h;
    format             = format_;

    init(pixels, makenormalmap, detailh);
}

texture::texture(
    unsigned w,
    unsigned h,
    int format_,
    mapping_mode mapping_,
    clamping_mode clamp,
    bool force_no_compression)
{
    dimension = GL_TEXTURE_2D;
    mapping   = mapping_;
    clamping  = clamp;

    if (!size_non_power_two())
    {
        if (w < 1 || (w & (w - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture width is no power of two!");
        }
        if (h < 1 || (h & (h - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture height is no power of two!");
        }
    }

    width = gl_width = w;
    height = gl_height = h;
    format             = format_;

    // error checks.
    if (mapping < 0 || mapping >= NR_OF_MAPPING_MODES)
    {
        THROW(texerror, get_name(), "illegal mapping mode!");
    }
    if (clamping < 0 || clamping >= NR_OF_CLAMPING_MODES)
    {
        THROW(texerror, get_name(), "illegal clamping mode!");
    }

    glGenTextures(1, &opengl_name);
    glBindTexture(GL_TEXTURE_2D, opengl_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapmodes[mapping]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter[mapping]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampmodes[clamping]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampmodes[clamping]);

    // enable anisotropic filtering if choosen
    if (use_anisotropic_filtering)
    {
        glTexParameterf(
            GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_level);
    }

    int internalformat = format;
    if (use_compressed_textures && !force_no_compression)
    {
        log_debug("Using compression, force =  " << force_no_compression);
        switch (format)
        {
            case GL_RGB:
                internalformat = GL_COMPRESSED_RGB_ARB;
                break;
            case GL_RGBA:
                internalformat = GL_COMPRESSED_RGBA_ARB;
                break;
            case GL_LUMINANCE:
                internalformat = GL_COMPRESSED_LUMINANCE_ARB;
                break;
            case GL_LUMINANCE_ALPHA:
                internalformat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB;
                break;
        }
    }

    // initialize texel data with empty pixels
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        internalformat,
        w,
        h,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        (void*) nullptr);
}

texture::texture(
    const std::string& filename,
    bool dummy,
    mapping_mode mapping_,
    clamping_mode clamp)
{
    dimension = GL_TEXTURE_2D;
    mapping   = mapping_;
    clamping  = clamp;

    // error checks.
    if (mapping < 0 || mapping >= NR_OF_MAPPING_MODES)
    {
        THROW(texerror, get_name(), "illegal mapping mode!");
    }
    if (clamping < 0 || clamping >= NR_OF_CLAMPING_MODES)
    {
        THROW(texerror, get_name(), "illegal clamping mode!");
    }

    dds_data image_data;
    load_dds(filename, image_data);

    width = gl_width = image_data.width;
    height = gl_height = image_data.height;

    int block_size;

    if (image_data.format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT)
    {
        block_size = 8;
    }
    else
    {
        block_size = 16;
    }

    glGenTextures(1, &opengl_name);
    glBindTexture(GL_TEXTURE_2D, opengl_name);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mapmodes[mapping]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magfilter[mapping]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, clampmodes[clamping]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, clampmodes[clamping]);

    // enable anisotropic filtering if choosen
    if (use_anisotropic_filtering)
    {
        glTexParameterf(
            GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_level);
    }

    int m_size, m_offset = 0, m_width = width, m_height = height;

    // Load the mip-map levels
    for (int i = 0; i < image_data.numMipMaps; ++i)
    {
        if (m_width == 0)
        {
            m_width = 1;
        }
        if (m_height == 0)
        {
            m_height = 1;
        }

        m_size = ((m_width + 3) / 4) * ((m_height + 3) / 4) * block_size;

        glTexImage2D(
            GL_TEXTURE_2D,
            i,
            image_data.format,
            m_width,
            m_height,
            0,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr);
        glCompressedTexSubImage2DARB(
            GL_TEXTURE_2D,
            i,
            0,
            0,
            m_width,
            m_height,
            image_data.format,
            m_size,
            &image_data.pixels[m_offset]);

        m_offset += m_size;

        // Half the image size for the next mip-map level...
        m_width  = (m_width / 2);
        m_height = (m_height / 2);
    }
}

texture::~texture()
{
#ifdef MEMMEASURE
    unsigned sub_mem_used = gl_width * gl_height * get_bpp();
    if (do_mipmapping[mapping])
        sub_mem_used = (4 * sub_mem_used) / 3;
    mem_used -= sub_mem_used;
    log_debug(
        "Freed " << sub_mem_used << " bytes of video memory for texture '"
                 << texfilename << "', total video mem use " << mem_used / 1024
                 << " kb");
    mem_freed += sub_mem_used;
    log_debug("Video mem usage " << mem_alloced << " vs " << mem_freed);
#endif
    glDeleteTextures(1, &opengl_name);
}

void texture::sub_image(
    int xoff,
    int yoff,
    unsigned w,
    unsigned h,
    const std::vector<uint8_t>& pixels,
    int format_)
{
    glBindTexture(GL_TEXTURE_2D, opengl_name);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0 /* mipmap level */,
        xoff,
        yoff,
        w,
        h,
        format_,
        GL_UNSIGNED_BYTE,
        &pixels[0]);
}

void texture::sub_image(
    const sdl_image& sdlimage,
    int xoff,
    int yoff,
    unsigned w,
    unsigned h)
{
    SDL_Surface* teximage = sdlimage.get_SDL_Surface();

    SDL_LockSurface(teximage);

    const SDL_PixelFormat& fmt = *(teximage->format);
    unsigned bpp               = fmt.BytesPerPixel;

    vector<uint8_t> data;
    if (fmt.palette != nullptr)
    {
        // no color tables, fixme
        return;
    }
    bool usealpha = fmt.Amask != 0;
    if (usealpha)
    {
        format = GL_RGBA;
        bpp    = 4;
    }
    else
    {
        format = GL_RGB;
        bpp    = 3;
    }
    data.resize(w * h * bpp);
    unsigned char* ptr    = &data[0];
    unsigned char* offset = (static_cast<unsigned char*>(teximage->pixels))
                            + yoff * teximage->pitch + xoff * bpp;
    for (unsigned y = 0; y < h; y++)
    {
        // old code, that assumes bytes come in R,G,B order:
        memcpy(ptr, offset, w * bpp);
        offset += teximage->pitch;
        ptr += w * bpp;
    }
    SDL_UnlockSurface(teximage);

    glBindTexture(GL_TEXTURE_2D, opengl_name);
    glTexSubImage2D(
        GL_TEXTURE_2D,
        0 /* mipmap level */,
        xoff,
        yoff,
        w,
        h,
        format,
        GL_UNSIGNED_BYTE,
        &data[0]);
}

auto texture::get_bpp() const -> unsigned
{
    switch (format)
    {
        case GL_RGB:
            return 3;
        case GL_RGBA:
            return 4;
        case GL_LUMINANCE:
            return 1;
        case GL_LUMINANCE_ALPHA:
            return 2;
        default:
            ostringstream oss;
            oss << "unknown texture format " << format << "\n";
            THROW(texerror, get_name(), oss.str());
    }
    return 4;
}

void texture::set_gl_texture() const
{
    glBindTexture(GL_TEXTURE_2D, get_opengl_name());
}

// draw_image
void texture::draw(int x, int y, const colorf& col) const
{
    draw(x, y, width, height, col);
}

void texture::draw_hm(int x, int y, const colorf& col) const
{
    draw_hm(x, y, width, height, col);
}

void texture::draw_vm(int x, int y, const colorf& col) const
{
    draw_vm(x, y, width, height, col);
}

void texture::draw(int x, int y, int w, int h, const colorf& col) const
{
    float u = float(width) / gl_width;
    float v = float(height) / gl_height;
    primitives::textured_quad(
        vector2f(x, y),
        vector2f(x + w, y + h),
        *this,
        vector2f(0, 0),
        vector2f(u, v),
        col)
        .render();
}

void texture::draw_hm(int x, int y, int w, int h, const colorf& col) const
{
    float u = float(width) / gl_width;
    float v = float(height) / gl_height;
    primitives::textured_quad(
        vector2f(x, y),
        vector2f(x + w, y + h),
        *this,
        vector2f(u, 0),
        vector2f(0, v),
        col)
        .render();
}

void texture::draw_vm(int x, int y, int w, int h, const colorf& col) const
{
    float u = float(width) / gl_width;
    float v = float(height) / gl_height;
    primitives::textured_quad(
        vector2f(x, y),
        vector2f(x + w, y + h),
        *this,
        vector2f(0, v),
        vector2f(u, 0),
        col)
        .render();
}

void texture::draw_rot(int x, int y, double angle, const colorf& col) const
{
    draw_rot(x, y, angle, get_width() / 2, get_height() / 2, col);
}

void texture::draw_rot(
    int x,
    int y,
    double angle,
    int tx,
    int ty,
    const colorf& col) const
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 0, 1);
    draw(-tx, -ty, col);
    glPopMatrix();
}

void texture::draw_tiles(int x, int y, int w, int h, const colorf& col) const
{
    float tilesx = float(w) / gl_width;
    float tilesy = float(h) / gl_height;
    primitives::textured_quad(
        vector2f(x, y),
        vector2f(x + w, y + h),
        *this,
        vector2f(0, 0),
        vector2f(tilesx, tilesy),
        col)
        .render();
}

void texture::draw_tiles_rot(
    int x,
    int y,
    int w,
    int h,
    double angle,
    const colorf& col) const
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 0, 1);
    draw_tiles(-w, -h, 2 * w, 2 * h, col);
    glPopMatrix();
}

void texture::draw_subimage(
    int x,
    int y,
    int w,
    int h,
    unsigned tx,
    unsigned ty,
    unsigned tw,
    unsigned th,
    const colorf& col) const
{
    float x1 = float(tx) / gl_width;
    float y1 = float(ty) / gl_height;
    float x2 = float(tx + tw) / gl_width;
    float y2 = float(ty + th) / gl_height;
    primitives::textured_quad(
        vector2f(x, y),
        vector2f(x + w, y + h),
        *this,
        vector2f(x1, y1),
        vector2f(x2, y2),
        col)
        .render();
}

auto texture::get_max_size() -> unsigned
{
    GLint i;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &i);
    return i;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// texture3d
//
////////////////////////////////////////////////////////////////////////////////////////////////////

texture3d::texture3d(
    const std::vector<uint8_t>& pixels,
    unsigned w,
    unsigned h,
    unsigned d,
    int format_,
    mapping_mode mapping_,
    clamping_mode clamp)
{
    mapping  = mapping_;
    clamping = clamp;

    if (!size_non_power_two())
    {
        if (w < 1 || (w & (w - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture width is no power of two!");
        }
        if (h < 1 || (h & (h - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture height is no power of two!");
        }
        if (d < 1 || (d & (d - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture depth is no power of two!");
        }
    }

    width = gl_width = w;
    height = gl_height = h;
    depth = gl_depth = d;
    format           = format_;

    if (mapping < 0 || mapping >= NR_OF_MAPPING_MODES)
    {
        THROW(texerror, get_name(), "illegal mapping mode!");
    }
    if (clamping < 0 || clamping >= NR_OF_CLAMPING_MODES)
    {
        THROW(texerror, get_name(), "illegal clamping mode!");
    }

    unsigned ms = get_max_size();
    if (width > ms || height > ms || depth > ms)
    {
        THROW(
            texerror,
            "3d tex",
            "texture values too large, not supported by card");
    }

    glGenTextures(1, &opengl_name);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_3D, opengl_name);

    // make gl texture
    int internalformat = format;
    if (use_compressed_textures)
    {
        switch (format)
        {
            case GL_RGB:
                internalformat = GL_COMPRESSED_RGB_ARB;
                break;
            case GL_RGBA:
                internalformat = GL_COMPRESSED_RGBA_ARB;
                break;
            case GL_LUMINANCE:
                internalformat = GL_COMPRESSED_LUMINANCE_ARB;
                break;
            case GL_LUMINANCE_ALPHA:
                internalformat = GL_COMPRESSED_LUMINANCE_ALPHA_ARB;
                break;
        }
    }

    glTexImage3D(
        GL_TEXTURE_3D,
        0,
        internalformat,
        gl_width,
        gl_height,
        gl_depth,
        0,
        format,
        GL_UNSIGNED_BYTE,
        &pixels[0]);

    if (do_mipmapping[mapping])
    {
        // fixme: does this command set the base level, too?
        // i.e. are the two gl commands redundant?
        // fixme: doesn't work with textures that don't have power of two
        // size...

        // one is part of the GLU lib the other is probably part of GL
        // gluBuild3DMipmaps doesn't seem to work under Win32:
        //
        // texture.cpp(988) : error C3861: 'gluBuild3DMipmaps': identifier not
        // found
        //
        // --matt
#ifndef WIN32 // ugly hack to make it compile with win32, 3d textures not used
              // yet...
        gluBuild3DMipmaps(
            GL_TEXTURE_3D,
            format,
            gl_width,
            gl_height,
            gl_depth,
            format,
            GL_UNSIGNED_BYTE,
            &pixels[0]);
#endif
    }

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, mapmodes[mapping]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magfilter[mapping]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, clampmodes[clamping]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, clampmodes[clamping]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, clampmodes[clamping]);

    // enable anisotropic filtering if choosen
    if (use_anisotropic_filtering)
    {
        glTexParameterf(
            GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_level);
    }
    glBindTexture(GL_TEXTURE_3D, 0);
}

texture3d::texture3d(
    unsigned w,
    unsigned h,
    unsigned d,
    int format_,
    mapping_mode mapping_,
    clamping_mode clamp)
{
    mapping  = mapping_;
    clamping = clamp;

    if (!size_non_power_two())
    {
        if (w < 1 || (w & (w - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture width is no power of two!");
        }
        if (h < 1 || (h & (h - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture height is no power of two!");
        }
        if (d < 1 || (d & (d - 1)) != 0)
        {
            THROW(texerror, get_name(), "texture depth is no power of two!");
        }
    }

    width = gl_width = w;
    height = gl_height = h;
    depth = gl_depth = d;
    format           = format_;

    if (mapping < 0 || mapping >= NR_OF_MAPPING_MODES)
    {
        THROW(texerror, get_name(), "illegal mapping mode!");
    }
    if (clamping < 0 || clamping >= NR_OF_CLAMPING_MODES)
    {
        THROW(texerror, get_name(), "illegal clamping mode!");
    }

    glGenTextures(1, &opengl_name);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindTexture(GL_TEXTURE_3D, opengl_name);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, mapmodes[mapping]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, magfilter[mapping]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, clampmodes[clamping]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, clampmodes[clamping]);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, clampmodes[clamping]);

    // enable anisotropic filtering if choosen
    if (use_anisotropic_filtering)
    {
        glTexParameterf(
            GL_TEXTURE_3D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropic_level);
    }

    glTexImage3D(
        GL_TEXTURE_3D,
        0,
        format,
        w,
        h,
        d,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        (void*) nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void texture3d::sub_image(
    int xoff,
    int yoff,
    int zoff,
    unsigned w,
    unsigned h,
    unsigned d,
    const std::vector<uint8_t>& pixels,
    int format_)
{
    glBindTexture(GL_TEXTURE_3D, opengl_name);
    glTexSubImage3D(
        GL_TEXTURE_3D,
        0 /* mipmap level */,
        xoff,
        yoff,
        zoff,
        w,
        h,
        d,
        format_,
        GL_UNSIGNED_BYTE,
        &pixels[0]);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void texture3d::draw(
    int x,
    int y,
    int w,
    int h,
    const vector3f& tc0,
    const vector3f& tcdx,
    const vector3f& tcdy) const
{
    // glEnable(GL_TEXTURE_3D); // important! let caller do this...
    float data[4 * (2 + 3)];
    data[0 * 5 + 0] = x;
    data[0 * 5 + 1] = y + h;
    (tc0 + tcdy).to_mem(&data[0 * 5 + 2]);
    data[1 * 5 + 0] = x + w;
    data[1 * 5 + 1] = y + h;
    (tc0 + tcdy + tcdx).to_mem(&data[1 * 5 + 2]);
    data[2 * 5 + 0] = x + w;
    data[2 * 5 + 1] = y;
    (tc0 + tcdx).to_mem(&data[2 * 5 + 2]);
    data[3 * 5 + 0] = x;
    data[3 * 5 + 1] = y;
    tc0.to_mem(&data[3 * 5 + 2]);
    glBindTexture(GL_TEXTURE_2D, 0); // important
    glBindTexture(GL_TEXTURE_3D, opengl_name);
    glVertexPointer(2, GL_FLOAT, 4 * 5, &data[0]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(3, GL_FLOAT, 4 * 5, &data[2]);
    uint8_t idx[4] = {0, 1, 2, 3};
    glDrawRangeElements(GL_QUADS, 0, 3, 4, GL_UNSIGNED_BYTE, &idx[0]);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindTexture(GL_TEXTURE_3D, 0);
}
