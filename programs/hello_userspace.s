# fork
mov $2, %eax
int $80
# exit syscall
#mov $1, %eax
mov $0xdeadbabe, %ebx
mov $0xdead, %ecx
mov $0xbabe, %ecx
mov $0xde, %edx
mov $0xad, %esi
mov $0xbb, %edi
int $80
