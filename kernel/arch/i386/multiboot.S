.set MULTIBOOT2_HEADER_MAGIC,      0xE85250D6
.set MULTIBOOT2_ARCHITECTURE_I386, 0
.set MULTIBOOT2_HEADER_LEN,        multiboot_header_end - multiboot_header_start
.set CHECKSUM,                     -(MULTIBOOT2_HEADER_MAGIC + MULTIBOOT2_ARCHITECTURE_I386 + MULTIBOOT2_HEADER_LEN)
.set MULTIBOOT2_HEADER_TAG_END,    0
.set FLAGS,                        0

# Multiboot2 header
# See https://www.gnu.org/software/grub/manual/multiboot2/multiboot.html#OS-image-format
.section .multiboot
multiboot_header_start:
.long MULTIBOOT2_HEADER_MAGIC
.long MULTIBOOT2_ARCHITECTURE_I386
.long MULTIBOOT2_HEADER_LEN
.long CHECKSUM
# Multiboot2 header tags:
.short MULTIBOOT2_HEADER_TAG_END
.short FLAGS
.long 8
multiboot_header_end:
