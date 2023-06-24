<p align="center">
	<img src="logo.png">
</p>

# libpwu

### ABOUT:

Libpwu is a process exploitation library that offers an organised & structured view of processes and their memory. Libpwu unifies the most useful components of <i>ptrace</i>, <i>dlfcn</i> and other Linux utilities under a single interface. The library provides a range of capabilities from declarative calls to precise, low level operations that automate only the most tedious parts. High level functions are made to be extensible and modular. libpwu aims to provide the user with the ability to seamlessly intertwine high level functions with the user's own low level implementations through the modular and extensible nature of its components.

Libpwu includes features for:
- Function hooking.
- Payload injection & mutation.
- Cave enumeration.
- Starting new threads without libc or pthreads.
- Dynamically linked symbol resolution.
- Pattern searching.
- Structured view of process memory maps.
- Plus many quality of life utilities.

Interested? Take a look at the "Getting started" section below.

---

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
# make install
```

If you would like to install additional markdown documentation:
```
# make install_docs
```

---

### LINKING:

Compilers typically include libc by default when compiling. Since libpwu is not part of libc, you'll have to tell the compiler to link the library manually. Here is what that would look like:

```
$ gcc -o prog prog.c -lpwu
```

The <i>-l</i> option takes the name of the library minus the 'lib' prefix and the '.so' extension. In the sources for your program, include the library just as you would include any part of libc:

```
#include <libpwu.h>
```

---

### GETTING STARTED:

To get started, take a look at <i>docs/markdown/intro.md</i> for quick introduction to commonly used components of Libpwu. Example implementations of payload injection, function hooking, symbol resolution and more can be found in <i>examples</i>.

For documentation, see <i>docs</i>. Documentation is available in markdown and roff (manpage) formats. I recommend starting with a read of <i>structs.md</i>. The default installation will install manpages only; to install markdown docs run <i>make install_docs</i>
