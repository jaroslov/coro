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


// Inspired by:
//  http://fanf.livejournal.com/105413.html

#include "coro.hpp"

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

struct costk
{
    jmp_buf     entry;
    void*       scratch;
    void*       stack;
    size_t      stacklen;
    jmp_buf     recurse10;
    int         counter;
};

void* getstack(costk& stk)
{
    return stk.stack;
}

size_t getstacklen(costk& stk)
{
    return stk.stacklen;
}

void*& getscratch(costk& stk)
{
    return stk.scratch;
}

jmp_buf& getentry(costk& stk)
{
    return stk.entry;
}

void recurse10(costk& stk, const char*& name, int& x)
{
    ++stk.counter;
    fprintf(stdout, "Name: %s\n", name);
    x = 1001;
    coto(stk.recurse10, stk.entry);

    ++stk.counter;
    fprintf(stdout, "Name: %s\n", name);

    return coret(stk);
}

int main(int argc, char *argv[])
{
    const char* name    = "ben";
    int messageback     = 10;

    costk r10   = {
        .entry         = { },
        .stack          = calloc(1, 16 * 1024),
        .stacklen       = 16 * 1024,
        .recurse10      = { },
        .counter        = 0,
    };
    (void)r10;

    fprintf(stdout, "Back: %d\n", messageback);
    coro(r10, recurse10, r10, name, messageback);
    ++r10.counter;

    name                = "jon";

    fprintf(stdout, "Back: %d\n", messageback);
    coto(r10.entry, r10.recurse10);
    ++r10.counter;

    fprintf(stdout, "Cntr: %d\n", r10.counter);

    free(r10.stack);

    return 0;
}
