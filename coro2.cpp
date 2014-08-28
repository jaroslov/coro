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
#include <cstdio>
#include <cstdlib>

struct Billy { virtual ~ Billy() {} int K; };
struct Johnny : Billy { virtual ~ Johnny() { fprintf(stdout, "%s\n", "Johnny"); } };

struct Dealloca { ~ Dealloca() { fprintf(stdout, "%s\n", "Dealloca?"); } };

enum
{
    co_main     = 0,
    co_printk   = 1,
};

Billy* printk(costk<>& stk, const char*& name)
{
    Dealloca            D{};
    fprintf(stdout, "Name: %s\n", name);

    const char* newname = 0;
    cotie(newname)      = coto(stk[co_printk], stk[co_main]);
    fprintf(stdout, "Newname: %s\n", newname);

    int anumber         = 0;
    cotie(anumber)      = coto(stk[co_printk], stk[co_main], 1001, 10);
    fprintf(stdout, "... and ... %d\n", anumber);

    Billy *B            = new Johnny();
    B->K                = -15;

    return coret(stk[co_main], B);
}

int main(int argc, char *argv[])
{
    costk<> stk         = {};

    const char* name    = 0;
    name                = "ben";

    coro(stk[co_main], printk, stk, name);

    int anum = 0, bnum = 0;
    cotie(anum, bnum)   = coto(stk[co_main], stk[co_printk], "jerry");
    fprintf(stdout, "Backnum: %d %d\n", anum, bnum);

    Billy* B;
    cotie(B)            = coto(stk[co_main], stk[co_printk], 10000);
    fprintf(stdout, "Billy: %d\n", B->K);

    delete B;

    return 0;
}
