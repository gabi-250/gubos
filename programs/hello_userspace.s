mov $0xdeadbabe, %eax
# Trigger a general protection fault
cli
jmp .
