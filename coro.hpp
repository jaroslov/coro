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

#ifndef CORO_HPP
#define CORO_HPP

#include <cassert>
#include <csetjmp>
#include <tuple>
#include <vector>

#define cosetjmp(X) _setjmp(X)
#define colongjmp(X, Y) _longjmp(X, Y)

template <int Len=16 * 1024> struct costk;

template <int Len=16 * 1024>
struct cojumper
{
    costk<Len>* stk;
    size_t      idx;

    template <typename... Args>
    void set(Args&&... args)
    {
        this->stk->template set(this->idx, std::forward<Args>(args)...);
    }

    template <typename... Args>
    std::tuple<Args...>& get()
    {
        return this->stk->template get<Args...>(this->idx);
    }

    operator jmp_buf&()
    {
        return this->stk->channel[this->idx].point;
    }
};

template <int Len>
struct costk
{
    struct codata { virtual ~ codata() {} };
    template <typename... Args>
    struct codatat : codata
    {
        codatat(Args... args) : data(args...) {}
        virtual ~ codatat() {}
        std::tuple<Args...>     data;
    };
    struct cojmp { jmp_buf point; };
    struct cochnl { std::vector<uint8_t> data; jmp_buf point; };

    void*                   scratch;
    size_t                  ret;
    std::vector<uint8_t>    stack;
    std::vector<cochnl>     channel;

    cojumper<Len> operator[] (size_t idx)
    {
        if (this->channel.size() <= idx)
        {
            this->channel.resize(idx + 1);
        }
        cojumper<Len> cj = { this, idx };
        return cj;
    }

    template <typename... Args>
    void set(size_t idx, Args... args)
    {
        if (!this->channel[idx].data.empty())
        {
            reinterpret_cast<codata*>(&this->channel[idx].data[0])->~codata();
        }
        this->channel[idx].data.resize(sizeof(codatat<Args...>));
        new (reinterpret_cast<void*>(&this->channel[idx].data[0])) codatat<Args...>(args...);
    }

    template <typename... Args>
    std::tuple<Args...>& get(size_t idx)
    {
        return reinterpret_cast<codatat<Args...>*>(&this->channel[idx].data[0])->data;
    }
};

template <int Len>
inline void* getstack(cojumper<Len>& stk)
{
    if (stk.stk->stack.empty())
    {
        stk.stk->stack.resize(Len);
    }
    return &stk.stk->stack[0];
}

template <int Len>
inline size_t getstacklen(cojumper<Len>& stk)
{
    return stk.stk->stack.size();
}

template <int Len>
inline void*& getscratch(cojumper<Len>& stk)
{
    return stk.stk->scratch;
}

template <int Len>
inline void setret(cojumper<Len>& stk)
{
    stk.stk->ret    = stk.idx;
}

template <int Len>
inline cojumper<Len>& asret(cojumper<Len>& stk)
{
    stk.idx         = stk.stk->ret;
    return stk;
}

template <typename... Args>
struct cotier
{
    cotier(Args&... args) : tie(args...) {}
    cotier& operator=(cotier const& co)
    {
        this->tie   = co.tie;
        return *this;
    }
    template <int Len>
    void operator=(cojumper<Len> stk)
    {
        this->tie   = stk.template get<Args...>();
    }

    std::tuple<Args&...>    tie;
};

template <typename... Args>
inline cotier<Args...> cotie(Args&... args)
{
    return cotier<Args...>(args...);
}

template <typename FromStk, typename ToStk, typename... Args>
inline FromStk coto(FromStk from, ToStk to, Args&&... args)
{
    to.set(std::forward<Args>(args)...);
    if (cosetjmp(from)) return from;

    colongjmp(to, 1);

    __builtin_unreachable();
    abort();
}

template <typename ToStk, typename Arg>
inline Arg coret(ToStk to, Arg&& arg)
{
    setret(to);
    to.set(std::forward<Arg>(arg));
    return arg;
}

template <typename ToStk>
inline void coret(ToStk to)
{
    setret(to);
    return;
}

template <typename CoStk, typename Fun, typename... Brgs>
inline void coreturner(CoStk stk, Fun&& fun, Brgs&&... args)
{
    fun(args...);
    colongjmp(asret(stk), 1);

    __builtin_unreachable();
    abort();
}

template <typename CoStk, typename Fun, typename... Brgs>
inline void coro(CoStk stk, Fun&& fun, Brgs&&... args)
{
    void* stack     = getstack(stk);
    getscratch(stk) = alloca((uintptr_t)&stack - (uintptr_t)stack - getstacklen(stk));

    if (cosetjmp(stk)) return;
    coreturner(stk, fun, args...);

    __builtin_unreachable();
    abort();
}

#endif//CORO_HPP
