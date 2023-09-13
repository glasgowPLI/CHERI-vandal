# The Purecap Vandal (WIP)

***This library is an awful hack for containment testing purposes only. Using it is almost guaranteed to cause a crash.***

The Vandal tests the strength of containers on Morello by aggressively destroying any data it can achieve write
access to through an unsealed capability chain.

To invoke the Vandal, simply call the `vandalise()` function. The Vandal will use any capabilities left in registers,
within its PCC bounds, on the stack, or available via indirection to fill all reachable memory. An example snippet of
a hexdump after the attack would be:

```
0000aff0: dead dead dead dead 0000 0000 0000 0000
0000b000: dead dead dead dead 0000 0000 0000 0000
0000b010: dead dead dead dead 0000 0000 0000 0000
```

## Weaknesses

The Vandal assumes that registers c16 through c18 do not contain valid capabilities and will return an error code (-1)
if this assumption is false. It uses these registers to check the 'empty' stack for any stray capabilities before
allocating its stack space.

The Vandal will attack all reachable call frames on the stack above the call to `vandalise()`, and will attempt to
attack spent call frames from sibling calls, but will stop searching 'down' the empty stack when it finds 28
consecutive clear tags.

The Vandal cannot unseal capabilities, even where a valid unsealing capability is accessible.

If the Vandal finds a write-only capability to a memory region before finding a readable capability to the same
region, the memory will be overwritten before it can be searched for valid capabilities.



