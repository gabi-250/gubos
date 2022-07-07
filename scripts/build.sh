#!/bin/sh

set -eou pipefail

# Build all the example programs and get GRUB to load them into memory
mkdir -p grub/modules
(cd programs && make)
cp programs/* grub/modules

mkdir -p build/boot/grub

mkdir -p "$SYSROOT"
cp $SYSROOT/boot/gubos.kernel build/boot/gubos.kernel
cp grub/grub.cfg build/boot/grub/grub.cfg
cp -r grub/modules build/boot/grub/modules
grub-mkrescue -o gubos.bin build
