<p align="center">
	<img src="logo.png">
</p>

# libpwu

### ABOUT:

Libpwu is a process manipulation library that offers an organised & structured view of processes and their memory. Libpwu unifies the most useful components of <i>ptrace</i>, <i>dlfcn</i> and other Linux utilities under a single interface. The library provides a range of capabilities from declarative calls to precise, low level operations that automate only the most tedious parts. High level functions are made to be extensible and modular. libpwu aims to provide the user with the ability to seamlessly intertwine high level functions with the user's own low level implementations through the modular and extensible nature of its components.


### INSTALLATION:

Check the installation script to avoid any surprises. <i>make</i> is required to build libpwu. For most distributions it will be part of the <i>build-essentials</i> package or its equivalent.

Fetch the repo:
```
$ git clone https://github.com/vykt/libpwu
```

Build:
```
$ cd libpwu
$ make lib
```

Install:
```
# ./install.sh
```


### LINKING:

Compilers typically include libc by default when compiling. Since libpwu is not part of libc, you'll have to tell the compiler to link the library manually. Here is what that would look like:

```
$ gcc -o prog prog.c -lpwu
```

The <i>-l</i> option takes the name of the library minus the 'lib' prefix and the '.so' extension. In the sources for your program, include the library just as you would include any part of libc:

```
#include <libpwu.h>
```


### GETTING STARTED:

For a walkthrough and implementation of libpwu's core components, see <i>docs/markdown/intro.md</i>. For an overview of data structures and their related functions, see <i>docs/markdown/structs.md</i>. Example implementations are available in <i>examples</i>.


### CHECKLIST:

So far Libpwu supports the following functionalities:

- [x] Changing memory region protection by attaching to the process and hijacking 
      execution to call <i>mprotect()</i>.
- [x] Injecting objects.
- [x] Identifying memory caves.
- [x] Pattern searching.
- [x] Saving and restoring process registers.
- [x] Arbitrary reads & writes.
- [x] Various other small utilities that make life easier.
- [ ] Spinning up threads with <i>clone()</i>.
- [ ] Injecting shared objects.
- [ ] Bindings for rust.
- [ ] Installation script.
- [x] Documentation.
