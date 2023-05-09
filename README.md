<p align="center">
	<img src="logo.png">
</p>

# libpwu

### ABOUT:

Libpwu is a process manipulation library that offers an organised & structured 
view of memory that unifies the most useful components of `ptrace`, `dlfcn` and other 
Linux utilities under a single interface. The library provides a range of capabilities 
from declarative calls to precise, low level operations that automate only the most 
tedious parts.

Libpwu primarily deals with individual memory segments. The `read_maps()` call 
processes `/proc/pid/maps` and returns a structure containing an organised vector of 
these maps. Said maps can then be passed to analysis, injection or utility functions 
together with relevant arguments like offsets.


So far libpwu supports the following functionalities:

- Changing memory region protection by attaching to the process and hijacking execution 
 to call `mprotect()`.
- Injecting objects.
- Identifying memory caves.
- Pattern searching.
- Saving and restoring process registers.
- Arbitrary reads & writes.
- Various other small utilities that make life easier.


Plus the following planned features that are under development:

- Spinning up threads with `clone()`.
- Injecting shared objects.
- Bindings for rust.


Leave a star <3
