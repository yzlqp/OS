# It's for aarch64 architecture

# If the EC bit is 0B010101, the ISS field records as "ISS encoding for exception execution of HVC or SVC instruction (IMM16). 

# for SYS_fork:1
.global fork
fork:
	mov	x8, 1
	svc	0x0
	ret
# for SYS_exit:2
.global exit
exit:
	mov	x8, 2
	svc	0x0
	ret
# for SYS_wait:3
.global wait
wait:
	mov	x8, 3
	svc	0x0
	ret
# for SYS_pipe:4
.global pipe
pipe:
	mov	x8, 4
	svc	0x0
	ret
# for SYS_read:5
.global read
read:
	mov	x8, 5
	svc	0x0
	ret
# for SYS_kill:6
.global kill
kill:
	mov	x8, 6
	svc	0x0
	ret
# for SYS_exec:7
.global exec
exec:
	mov	x8, 7
	svc	0x0
	ret
# for SYS_fstat:8
.global fstat
fstat:
	mov	x8, 8
	svc	0x0
	ret
# for SYS_chdir:9
.global chdir
chdir:
	mov	x8, 9
	svc	0x0
	ret
# for SYS_dup:10
.global dup
dup:
	mov	x8, 10
	svc	0x0
	ret
# for SYS_getpid:11
.global getpid
getpid:
	mov	x8, 11
	svc	0x0
	ret
# for SYS_sbrk:12
.global sbrk
sbrk:
	mov	x8, 12
	svc	0x0
	ret
# for SYS_sleep:13
.global sleep
sleep:
	mov	x8, 13
	svc	0x0
	ret
# for SYS_uptime:14
.global uptime
uptime:
	mov	x8, 14
	svc	0x0
	ret
# for SYS_open:15
.global open
open:
	mov	x8, 15
	svc	0x0
	ret
# for SYS_write:16
.global write
write:
	mov	x8, 16
	svc	0x0
	ret
# for SYS_mknod:17
.global mknod
mknod:
	mov	x8, 17
	svc	0x0
	ret
# for SYS_unlink:18
.global unlink
unlink:
	mov	x8, 18
	svc	0x0
	ret
# for SYS_link:19
.global link
link:
	mov	x8, 19
	svc	0x0
	ret
# for SYS_mkdir:20
.global mkdir
mkdir:
	mov	x8, 20
	svc	0x0
	ret
# for SYS_close:21
.global close
close:
	mov	x8, 21
	svc	0x0
	ret
# for SYS_yield:22
.global yield
yield:
	mov	x8, 22
	svc	0x0
	ret
