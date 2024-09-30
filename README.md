<p align="center">
	<img src="logo.png">
</p>

# libpwu

![liblain](liblain.gif)

### ABOUT:

The Lain library (<i>liblain</i>) provides a programmatic interface to the memory and memory maps of processes on Linux. <i>Liblain</i> offers both a <i>procfs</i> and a <i>lainko</i> kernel driver backend. Both interfaces provide identical functionality. The kernel driver backend is not provided by this repository, please see https://github.com/vykt/lainko.

<i>Liblain</i> stores both VM areas and VM backing objects in lists, which means a memory map can be updated without invalidating pointers to the map. This allows liblain to be easily used for prolonged analysis where a target's memory allocations change. Despite being lists, traversal between relevant areas/objects is still possible in <i>O(1)</i> in many cases.

In addition to memory maps, <i>liblain</i> also provides the following:

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

Include <i>\<libpwu.h\></i> in your sources:
```
#include <liblain.h>
```

Ask your compiler to link liblain:
```
$ gcc -o test test.c -llain
```

---

### DOCUMENTATION:

See <i>doc/md/</i> for documentation. If you have installed <i>liblain</i>, you can view the manpages with <i>man 3 {liblain_error,liblain_iface,liblain_map,liblain_util}</i>. For examples, take a look at <i>src/test/</i>.
