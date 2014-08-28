/*

Copyright (c) 2014, Jacob N. Smith
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    1.  Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    2.  Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "coro.hpp"

#include <cinttypes>
#include <cstdio>
#include <cstring>

enum
{
    comain,
    cobmp,
    copxsrc,
};

struct BMP
{
    uint16_t    signature;
    uint32_t    filesize;
    uint16_t    reserved0;
    uint16_t    reserved1;
    uint32_t    offsetToData;
    uint32_t    bmpinfoheader;
    uint32_t    width;
    uint32_t    height;
    uint16_t    planes;
    uint16_t    bpp;
    uint32_t    compression;
    uint32_t    imagesize;
    uint32_t    hzreso;
    uint32_t    vtreso;
    uint32_t    colors;
    uint32_t    colorsimportant;
} __attribute__((packed));

void save_bmp(costk<>& stk, const char* fname)
{
    int width   = 0;
    int height  = 0;
    cotie(width, height)    = coto(stk[cobmp], stk[comain]);

    FILE* fp    = fopen(fname, "wb");
    if (!fp)
    {
        coret(stk[copxsrc], fp);
        return;
    }

    int paddedwidth = (width * 3 + 3)/4 * 4;

    BMP bmp;
    memset(&bmp, 0x0, sizeof(bmp));
    bmp.signature       = 0x4D42;
    bmp.filesize        = sizeof(struct BMP) + paddedwidth * height;
    bmp.offsetToData    = 54;
    bmp.bmpinfoheader   = 40;
    bmp.width           = width;
    bmp.height          = height;
    bmp.planes          = 1;
    bmp.bpp             = 24;
    bmp.compression     = 0;
    bmp.imagesize       = paddedwidth * height;
    bmp.hzreso          = 3800;
    bmp.vtreso          = 3800;
    bmp.colors          = 0;
    bmp.colorsimportant = 0;

    fwrite(&bmp, sizeof(BMP), 1, fp);
    uint8_t *pixels     = (uint8_t*)calloc(1, paddedwidth * height);

    bool has_pixels = false;
    cotie(has_pixels) = coto(stk[cobmp], stk[copxsrc], fp);

    int x           = 0;
    int y           = 0;
    uint32_t clr    = 0;
    while (has_pixels)
    {
        cotie(has_pixels)   = coto(stk[cobmp], stk[copxsrc]);
        if (!has_pixels) break;
        cotie(x, y)     = coto(stk[cobmp], stk[copxsrc]);
        cotie(clr)      = coto(stk[cobmp], stk[copxsrc]);
        uint32_t off    = paddedwidth * y;
        off             += x * 3;
        pixels[off+0]   = reinterpret_cast<uint8_t*>(&clr)[0];
        pixels[off+1]   = reinterpret_cast<uint8_t*>(&clr)[1];
        pixels[off+2]   = reinterpret_cast<uint8_t*>(&clr)[2];
    }

    fwrite(pixels, paddedwidth * height, 1, fp);
    fclose(fp);
    free(pixels);

    return coret(stk[copxsrc]);
}

void create_img(costk<>& stk, int width, int height)
{
    FILE* fp    = 0;
    cotie(fp)   = coto(stk[copxsrc], stk[cobmp], width, height);
    if (!fp) return;

    coto(stk[copxsrc], stk[cobmp], true);

    for (int yy = 0; yy < height; ++yy)
    {
        for (int xx = 0; xx < width; ++xx)
        {
            coto(stk[copxsrc], stk[cobmp], true);
            coto(stk[copxsrc], stk[cobmp], xx, yy);
            uint32_t color  = 0;
            double redy     = (height - yy) / double(height);
            double redx     = (width - xx) / double(width);
            double yred     = yy / double(height);
            double xred     = xx / double(width);
            color   |= (uint8_t)((redy * redx + yred * xred) * 255) << 16;
            color   |= (uint8_t)((yy / double(height)) * 255) << 8;
            color   |= (uint8_t)((xx / double(width))  * 255) << 0;
            coto(stk[copxsrc], stk[cobmp], color);
        }
    }

    coto(stk[copxsrc], stk[cobmp], false);

    return;
}

int main(int argc, char *argv[])
{
    const char* fname   = "my.bmp";
    uint32_t width      = 128;
    uint32_t height     = 128;
    if (argc > 1)
    {
        fname           = argv[1];
    }
    if (argc > 2)
    {
        width           = strtoull(argv[2], 0, 0);
        height          = width;
    }
    if (argc > 3)
    {
        height          = strtoull(argv[3], 0, 0);
    }

    costk<> stk{};

    coro(stk[comain], save_bmp, stk, fname);
    create_img(stk, width, height);

    fprintf(stdout, "%s\n", "FIN.");

    return 0;
}
