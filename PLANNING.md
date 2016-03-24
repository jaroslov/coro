PREFERRED INTERFACE
-------------------

    void ldbmp(int width, int height, cochnl pixdst)
    {
        auto B  = loadBMP();

        if (!coopen(pixdst))
            coyield();

        for (int YY = 0; YY < height; ++YY)
        {
            for (int XX = 0; XX < width; ++XX)
            {
                pixdst(XX, YY, getpixel(B, XX, YY));
            }
        }

        coclose(pixdst);
    }

    void svbmp(const char* filename, cochnl pixsrc)
    {
        auto B  = openFILE();

        while (coopen(pixsrc))
        {
            auto pix    = pixsrc();
            save(B, get<0>(pix), get<1>(pix), get<2>(pix));
        }
    }

    int main(int argc, char *argv[])
    {
        costk<> stack;
        cochnl  pix = mkcochnl<std::tuple<int, int, uint32_t>>(stack);
        coro cload  = mkcoro(stack, ldbmp, width, height, pix);
        coro csave  = mkcoro(stack, svbmp, "foo", pix);
        cobind(cload, csave, pix);

        cload();
        csave();

        return 0;
    }

SELECT
------

    int select(cochnl, <chnlTy>&, ...)

    std::tuple<int, int, uint32_t> pix;
    const char* metadata;
    switch (select(pixsrc, pix, metasrc, meta))
    {
    case 0: /* handle pixel input */ break;
    case 1; /* handle meta data */ break;
    }
