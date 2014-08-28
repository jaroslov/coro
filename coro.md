The `coro` library
------------------

This is an implementation of coroutines (with channels) in C++1y. This library makes the following assumptions about the C-machine model it is executing on:

1. The stack address space is 'above' the heap address space;
2. The stack is contiguous;
3. `alloca` just adjusts the stack pointer; and,
4. There are 'user space' `setjmp` and `longjmp` functions (on OSX & BSD: `_setjmp` and `_longjump`).

The `coro` library doesn't use an assembly (inline or otherwise), and is conformant C++1y.

Basic API overview
------------------

A coroutine consists of two parts:

1. A data structure to hold the state of the coroutine; and,
2. A set of functions to jump between the coroutine points.

In `coro` the data structure is called `costk<*>`. The `costk<*>` takes a single template parameter argument, which is the length of the stack to allocate in the heap.

    coro<> stk{}; // Create our stack.

The API consists of three functions:

1. `coro()` which defines a new coroutine, defines a local jump point, and calls the coroutine;
2. `coto()` which defines a local jump point, and jumps to another jump point; and,
3. `coret()` which returns (permanently) from a coroutine to the parent routine.

To create a coroutine, use the coroutine function `coro`:

    // void printname(coro<>& stk, const char*&);
    coro(stk, &printname, "billy");

The result of the call to `coro` will be to bind the function `printname` to the stack `stk` and then call `printname` with the arguments given in the call to `coro`; in this case, a c-string `"billy"`.

The standard `costk<*>` provided by the library automatically opens channels (identified by integers) between coroutines:

    int printname(costk<>& stk, const char*& name)
    {
        fprintf(stdout, "First Name: %s\n", name);
        cotie(name)         = coto(stk[0], stk[1]);
        fprintf(stdout, "Last name: %s\n", name);
        return coret(stk[1], 1);
    }

    costk<> stk         = {};
    coro(stk[1], printname, stk, (const char*)"billy");
    coto(stk[1], stk[0], (const char*)"bob");
    fprintf(stdout, "%s\n", "FIN.");

The function `coto` takes two channel markers: the first is the channel to return to, and the second is the channel to go to. In this case, `printname` marks the 'here' channel with the identifier `0`, and the 'there' channel with identifier `1`. The caller channel uses the opposite identifiers, i.e., `1` for 'here' and `0` for 'there'. The call to `coto` can take an optional channel message. The function `cotie` receives the result of a `coto` message. In this case, the message will be a pointer to the c-string "john".

Compiling with clang++, and running gives:

> clang++ -std=c++1y ex.cpp -g -o ex && ./ex

> First Name: billy

> Last name: bob

> FIN.

Unfinished business
-------------------

1. Pretty certain the perfect forwarding semantics are busted.
2. The use of `setjmp` and `longjmp` imply proper signal handling—this isn't the case.
3. The use of `asret` and `setret` is inelegant.

Advanced API overview
---------------------

It is possible to not use the `costk<*>` data structure and, instead, provide your own stack data-structure. The concept for a coroutine stack structure is:

Let `co` and `co2` be an instances of `costk`; let `fun` be some function; let `args...` be the set of arguments passed to `fun`; let `args2...` be a family of arguments that are coroutine re-entrant site specific, let e0, e1, ... be enumerations in an `enum`:

1. `colongjmp(co, 1)` jumps to the current coroutine point.
2. `cosetjmp(co)` marks the current coroutine point.
3. `coreturner(co, fun, args...)` safely returns (stack unwinding, stack clean-up, etc.) from a coroutine.
4. `co[e_i]` sets the current coroutine point to '`e_i`'.
5. `coro(co, co2, fun, args...)` sets the return point in the coroutine at the callsite of `coro` and enters into the function `fun`.
6. `coto(co, co2, args2...)` sets the current point in the coroutine at the site of `coto` and passes the arguments `args2...` to the coroutine point `co2`. The result of the called to `coto()` is
7. `cotie(args2...) = coto(...)` which binds the result of the call to `coto` to the arguments `args2...`.

The stack `costk<*>` also satisfies an internal API:

1. `getstack(co[e_i])` returns a pointer to the 'top' of the stack.
2. `getstacklen(co[e_i])` returns the length of the stack.
3. `getscratch(co[e_i])` returns a reference to a `void*`.
4. `setret(co[e_i])` sets the return point to `e_i`.
5. `asret(co[e_i])` returns to the point previous set by a call to `setret`, not `e_i`. The `asret` function will only be called when a coroutine exits permanently.
