# Runnable programs

This directory contains the programs we'll ask GRUB to load into memory. This
enables `memo` to access and "run" programs without having to implement a file
system.

For each new `<program.s>`, you will need to add a new `module2` command to
[grub.cfg](../grub/grub.cfg):

```
module2 /boot/grub/modules/<program>
```
