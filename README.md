<p align="center">
	<img src="logo.png">
</p>

# libpwu

### ABOUT:

Libpwu is a process manipulation library that offers an organised & structured 
view of memory and unifies the most useful components of `ptrace`, `dlfcn` and other 
Linux utilities under a single interface. The library provides a range of capabilities 
from declarative calls to precise, low level operations that automate only the most 
tedious parts.

Libpwu primarily deals with individual memory segments. The `read_maps()` call 
processes `/proc/pid/maps` and returns a structure containing an organised vector of 
these segments. Said segments can then be passed to analysis, injection or utility 
functions together with relevant arguments like offsets.

---

So far Libpwu supports the following functionalities:

- [x] Changing memory region protection by attaching to the process and hijacking 
      execution to call `mprotect()`.
- [x] Injecting objects.
- [x] Identifying memory caves.
- [x] Pattern searching.
- [x] Saving and restoring process registers.
- [x] Arbitrary reads & writes.
- [x] Various other small utilities that make life easier.
- [ ] Spinning up threads with `clone()`.
- [ ] Injecting shared objects.
- [ ] Bindings for rust.
- [ ] Documentation.
