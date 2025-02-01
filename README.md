# Lain

<p align="center">
    <img src="memcry.png" width="150" height="150">
</p>

### NOTE:

**liblain** is undergoing major testing and a minor refactor. Prebuilt binaries and static builds are coming. To get liblain working before the refactor is finished, install the latest release and [cmore v0.0.3](https://github.com/vykt/cmore/releases/tag/0.0.3).


### ABOUT:

The Lain library (<b>liblain</b>) provides a programmatic interface to the memory and memory maps of processes on Linux. Liblain can be used for:

- Code injection
- Memory analysis
- Anti-cheat bypass (see [lain.ko](https://github.com/vykt/lain.ko))

<b>liblain</b> offers both a procfs and a [lain.ko](https://github.com/vykt/lain.ko) LKM backend. Both interfaces provide identical functionality and are interchangable.

<b>liblain</b> stores both virtual memory areas and backing objects in nodes traversable as lists or trees. The use of nodes for storage means the internal memory map can be updated without invalidating any pointers. This makes development of complex tools much easier.

In addition to a memory interface <b>liblain</b> also provides several utilities including:

- Same method for resolving PID as ps/top.
- Fast address -> VM area search.

---

### DEPENDENCIES:

<b>liblain</b> requires [libcmore](https://github.com/vykt/libcmore).


### INSTALLATION:

Fetch the repo:
```
$ git clone https://github.com/vykt/liblain
```

Build:
```
$ make lib
```

Install:
```
# make install
```

Install additional markdown documentation:
```
# make install_doc
```

To uninstall:
```
# make uninstall
```

---

### LINKING:

Ensure your linker searches for liblain in the install directory (adjust as required):
```
# echo "/usr/local/lib" > /etc/ld.so.conf.d/liblain.conf
```

Include `<liblain.h\>` in your sources:
```
#include <liblain.h>
```

Ask your compiler to link liblain:
```
$ gcc -o test test.c -llain
```

---

### DOCUMENTATION:

See `./doc/md` for markdown documentation. After installing <b>liblain</b> the following manpages are available:

| Manpage         | Description                 |
| --------------- | --------------------------- |
| `liblain_error` | Error handling              |
| `liblain_map`   | Memory map data structure   |
| `liblain_iface` | Using interfaces            |
| `liblain_util`  | Utilities                   |
