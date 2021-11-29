#!/bin/sh

set -eou pipefail

build_libs() (
    mkdir -p "$SYSROOT"

    for PROJECT in $SYSTEM_HEADER_PROJECTS; do
        (cd $PROJECT && make install-headers)
    done

    for PROJECT in $PROJECTS; do
        (cd $PROJECT && make install)
    done
)

build_libs

mkdir -p build/boot/grub

cp sysroot/boot/gubos.kernel build/boot/gubos.kernel
cp grub/grub.cfg build/boot/grub/grub.cfg
grub-mkrescue -o gubos.bin build
