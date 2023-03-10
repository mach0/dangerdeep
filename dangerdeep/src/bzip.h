/*
 * Danger from the Deep - Open source submarine simulation
 * Copyright (C) 2003-2020  Thorsten Jordan, Luis Barrancos and others.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#pragma once

#include <bzlib.h>
#include <iostream>
#include <ostream>
#include <streambuf>
#include <vector>

#define BZ_NO_STDIO

class bzip_failure : public std::ios_base::failure
{
  private:
    int error_code;

  public:
    bzip_failure(int _error_code) : failure(""), error_code(_error_code){};
    [[nodiscard]] const char* what() const noexcept override
    {
        switch (error_code)
        {
            case BZ_SEQUENCE_ERROR:
                return ("BZ_SEQUENCE_ERROR");
                break;
            case BZ_PARAM_ERROR:
                return ("BZ_PARAM_ERROR");
                break;
            case BZ_MEM_ERROR:
                return ("BZ_MEM_ERROR");
                break;
            case BZ_DATA_ERROR:
                return ("BZ_DATA_ERROR");
                break;
            case BZ_DATA_ERROR_MAGIC:
                return ("BZ_DATA_ERROR_MAGIC");
                break;
            case BZ_IO_ERROR:
                return ("BZ_IO_ERROR");
                break;
            case BZ_UNEXPECTED_EOF:
                return ("BZ_UNEXPECTED_EOF");
                break;
            case BZ_OUTBUFF_FULL:
                return ("BZ_OUTBUFF_FULL");
                break;
            case BZ_CONFIG_ERROR:
                return ("BZ_CONFIG_ERROR");
                break;
            default:
                return ("BZ_UNKNOWN_ERROR");
        }
    };
    int get_error() { return error_code; }
};

class bzip_streambuf : public std::streambuf
{
  protected:
    std::ostream* outstream;
    std::istream* instream;
    std::vector<char> in_buffer, out_buffer;
    int blocksize_100_k, workfactor, buffer_size, state;
    bz_stream bzstream;
    const std::size_t put_back;
    enum
    {
        COMPRESS,
        DECOMPRESS,
        CLOSED
    } mode;

    int_type overflow(int_type ch) override;
    int_type underflow() override;
    int sync() override;

  private:
    void flush();
    int fill_buffer();
    int bzip2stream(char*, int);

    bzip_streambuf(const bzip_streambuf&) = delete;
    bzip_streambuf& operator=(const bzip_streambuf&) = delete;

  public:
    bzip_streambuf(std::ostream*, int, int, int);
    bzip_streambuf(std::istream*, int, int, int);
    void close()
    {
        if (mode == COMPRESS)
        {
            flush();
            state = BZ2_bzCompressEnd(&bzstream);
            if (state < 0)
                throw bzip_failure(state);
        }
        else if (mode == DECOMPRESS)
        {
            state = BZ2_bzDecompressEnd(&bzstream);
            if (state < 0)
                throw bzip_failure(state);
        }
        mode = CLOSED;
    }
};

class bzip_ostream : public std::ostream
{
  public:
    bzip_ostream(
        std::ostream* os,
        int blocksize  = 9,
        int workbuffer = 30,
        int buffsize   = 256) :
        std::ostream(new bzip_streambuf(os, blocksize, workbuffer, buffsize))
    {
    }
    ~bzip_ostream() override { close(); }
    void close() { ((bzip_streambuf*) rdbuf())->close(); }
};

class bzip_istream : public std::istream
{
  public:
    bzip_istream(std::istream* is, int buffsize = 256, int small = 0) :
        std::istream(new bzip_streambuf(is, buffsize, 8, small))
    {
    }
    ~bzip_istream() override { close(); }
    void close() { ((bzip_streambuf*) rdbuf())->close(); }
};
