# Debug Break

[debugbreak.h](https://github.com/scottt/debugbreak/blob/master/debugbreak.h) allows you to put breakpoints in your C/C++ code with a call to **debug_break()**:
```C
#include <stdio.h>
#include "debugbreak.h"

int main()
{
	debug_break();
	printf("hello world\n");
	return 0;
}
```
This works like the **DebugBreak()** intrinsic provided by [Windows](http://msdn.microsoft.com/en-us/library/ea9yy3ey.aspx) and [QNX](http://www.qnx.com/developers/docs/6.3.0SP3/neutrino/lib_ref/d/debugbreak.html).
Currently it supports gcc and Clang, works well on ARM, i686, x86-64 and has a fallback code path for other architectures.

Just include the header in your C/C++ code.

**License**: [2-Clause BSD](https://github.com/scottt/debugbreak/blob/master/COPYING).

Implementation Notes
================================

The requirements for the **debug_break()** function are:
* Act as a compiler code motion barrier
* Don't cause the compiler optimizers to think that the code following it can be removed
* Trigger a software breakpoint hit when executed (i.e. **SIGTRAP** on Linux)
* **continue**, **next**, **step**, **stepi** GDB commands work after a **debug_break()** hit

Ideally, both gcc and Clang would provide a **__builtin_debugger()** that satisfies the above on all supported architectures and operating systems.
Unfortunately, we're not there yet.
gcc's [__builtin_trap()](http://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html#index-g_t_005f_005fbuiltin_005ftrap-3278) causes the optimizers to think the code follwing can be removed ([test/trap.c](https://github.com/scottt/debugbreak/blob/master/test/trap.c)):
```C
#include <stdio.h>

int main()
{
	__builtin_trap();
	printf("hello world\n");
	return 0;
}
```
```
main
0x0000000000400390 <+0>:     0f 0b	ud2    
```
**__builtin_trap()** also generates an **ud2** instruction which triggers **SIGILL** instead of **SIGTRAP** on i386 / x86-64.
This makes it necessary to change GDB's default behavior on **SIGILL** to not terminate the inferior:
```
(gdb) handle SIGILL stop nopass
```
Even after this, continuing execution in GDB doesn't work well on some GCC, GDB combinations.

On ARM, **__builtin_trap()** generates a call to **abort()**.

**debug_break()** generates an **int3** instruction on i386 / x86-64 ([test/break.c](https://github.com/scottt/debugbreak/blob/master/test/break.c)):
```C
#include <stdio.h>
#include "debugbreak.h"
   
int main()
{
	debug_break();
	printf("hello world\n");
	return 0;
}
```
```
main
0x00000000004003d0 <+0>:     50	push   %rax
0x00000000004003d1 <+1>:     cc	int3   
0x00000000004003d2 <+2>:     bf a0 05 40 00	mov    $0x4005a0,%edi
0x00000000004003d7 <+7>:     e8 d4 ff ff ff	callq  0x4003b0 <puts@plt>
0x00000000004003dc <+12>:    31 c0	xor    %eax,%eax
0x00000000004003de <+14>:    5a	pop    %rdx
0x00000000004003df <+15>:    c3	retq   
```
Which correctly trigges **SIGTRAP** and stepping in GDB after a **debug_break()** hit works well.
Clang / LLVM also has a **__builtin_trap()** that generates **ud2** but further provides [__builtin_debugger()](http://lists.cs.uiuc.edu/pipermail/llvm-commits/Week-of-Mon-20120507/142621.html) that generates **int3** on i386 / x86-64.

On ARM, **debug_break()** generates **.inst 0xe7f001f0** in ARM mode and **.inst 0xde01** in Thumb mode which correctly triggers *SIGTRAP* on Linux. Unfortunately, stepping in GDB after a **debug_break()** hit doesn't work and requires a workaround like:
```
(gdb) set $l = 2
(gdb) tbreak *($pc + $l)
(gdb) jump   *($pc + $l)
(gdb) # Change $l from 2 to 4 for ARM mode
```
to jump over the instruction.
A new GDB command, **debugbreak-step**, is defined in [debugbreak-gdb.py](https://github.com/scottt/debugbreak/blob/master/debugbreak-gdb.py) to automate the above.
```
$ arm-none-linux-gnueabi-gdb -x debugbreak-gdb.py test/break-c++
<...>
(gdb) run
Program received signal SIGTRAP, Trace/breakpoint trap.
main () at test/break-c++.cc:6
6		debug_break();

(gdb) debugbreak-step

7		std::cout << "hello, world\n";
```

On other architectures, **debug_break()** generates a call to **raise(SIGTRAP)**.
