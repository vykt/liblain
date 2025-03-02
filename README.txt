## TL;DR





Memcry is a memory manipulation library. It's des

Memcry provides two views of a target's memory: areas


`mc_vm_map` can be updated at any time by calling `mc_update_map()`.
Most importantly, updating the map does not invalidate any existing
pointers. Instead any areas discovered to no longer be mapped are
moved to unmapped lists inside `mc_vm_map`, and their `mapped` flags
are set to `false`.


## Interfaces:

As an obfuscation measure your target may be watching for memory
accesses through some system APIs. Memcry performs all operations
through _interfaces_. An interface provides read & write primitives, and
a method to acquire the target's memory map. The library comes with support
for two interfaces:

- procfs (included)
- krncry (WIP kernel module, get [here](https://github.com/vykt/krncry))


## Utils:

Memcry also offers some QoL utils:

- Finding the PID of a target by name the exact way ps & top do.
- A fast address -> area/object search.
- Offset & bound offset getters.







For any questions, contact me on discord (@vykt), by
email (vykt[at]disroot[dot]org), or on LiberaIRC (@vykt).


The core memcry data structure (`mc_vm_map`) 

Searching
