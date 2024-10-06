# liblain

<p align="center">
    <img src="liblain.png">
</p>

### ABOUT:

The Lain library (<b>liblain</b>) provides a programmatic interface to the memory and memory maps of processes on Linux. <b>Liblain</b> offers both a <b>procfs</b> and a [lainko](https://github.com/vykt/lainko) kernel driver backend. Both interfaces provide identical functionality. The kernel driver backend is not provided by this repository.

<b>Liblain</b> stores both VM areas and VM backing objects in lists, which means a memory map can be updated without invalidating pointers to the map. This allows liblain to be easily used for prolonged analysis where a target's memory allocations change. Despite being lists, traversal between relevant areas/objects is still possible in <b>O(1)</b> in many cases.

In addition to memory maps, <b>liblain</b> also provides the following:

- Read / write process memory.
- Resolve a process name to PID(s) the same way utilities like ps/top/htop do.
- Various utils to streamline the development process.

---

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
# make install_docs
```

---

### LINKING:

Ensure your linker searches for liblain in the install directory (adjust as required):
```
# echo "/usr/local/lib" > /etc/ld.so.conf.d/liblain.conf
```

Include <b>\<libpwu.h\></b> in your sources:
```
#include <liblain.h>
```

Ask your compiler to link liblain:
```
$ gcc -o test test.c -llain
```

---

### DOCUMENTATION:

See <b>/doc/md</b> for documentation. If you have installed <b>liblain</b>, you can view the manpages with <b>man 3 {liblain_error,liblain_iface,liblain_map,liblain_util}</b>. For examples, take a look at <b>/src/test</b>.
