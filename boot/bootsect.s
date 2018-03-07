.code16

SYSSIZE=0x3000
SETUPLEN=4
BOOTSEG=0x07c0
INITSEG=0x9000
SETUPSEG=0x9020
SYSSEG=0x1000
ENDSEG=SYSSEG+SYSSIZE
ROOT_DEV=0x306


.text
.globl
.global start
start:
  movw $BOOTSEG,%ax
  movw %ax,%ds
  movw $INITSEG,%ax
  movw %ax,%es
  movw $256,%cx
  subw %si,%si
  subw %di,%di
  cld
  rep
  movsw
  ljmp $INITSEG,$go

go:
  movw %cs,%ax
  movw %ax,%ds
  movw %ax,%es
  movw %ax,%ss
  movw $0xff00,%sp

load_setup:
  movw $0x0000,%dx
  movw $0x0002,%cx
  movw $0x0200,%bx
  movw $SETUPSEG+0x200,%ax
  int $0x13
  jnc ok_load_setup
  movw $0x0000,%dx
  movw $0x0000,%ax
  int $0x13
  jb load_setup
# jmp
ok_load_setup:
  movb $0x00,%dl
  movw $0x0800,%ax
  int $0x13
  movb $0x00,%ch
  movw %cx,%cs:sectors
  movw $INITSEG,%ax
  movw %ax,%es
  movb $0x03,%ah
  xorb %bh,%bh
  int $0x10

  movw $24,%cx
  movw $0x0007,%bx
  movw $msg1,%bp
  movw $0x1301,%ax
  int $0x10

  movw $SYSSEG,%ax
  movw %ax,%es
  call read_it
  call kill_motor

  movw %cs:root_dev,%ax
  cmp $0,%ax
  jne root_defined
  movw %cs:sectors+0,%bx
  movw $0x0208,%ax
  cmp $15,%bx
  je root_defined
  movw $0x021c,%ax
  cmp $18,%bx
  je root_defined

undef_root:
  jmp undef_root

root_defined:
  movw %ax,%cs:root_dev
  ljmp $SETUPSEG,$0

sread:  .word 1+SETUPLEN
head: .word 0
track:  .word 0

read_it:
  movw %es,%ax
  test $0x0fff,%ax

die:
  jne die
  xorw %bx,%bx

rp_read:
  movw %es,%ax
  cmp $ENDSEG,%ax
  jb ok1_read
  ret

ok1_read:
  movw %cs:sectors+0,%ax
  subw sread,%ax
  movw %ax,%cx
  shl $9,%cx
  addw %bx,%cx
  jnc ok2_read
  je ok2_read
  xorw %ax,%ax
  subw %bx,%ax
  shr $9,%ax

ok2_read:
  call read_track
  movw %ax,%cx
  addw sread,%ax
  cmpw %cs:sectors+0,%ax
  jne ok3_read
  movw $1,%ax
  sub head,%ax
  jne ok4_read
  jnc track

ok4_read:
  movw %ax,head
  xorw %ax,%ax

ok3_read:
  movw %ax,sread
  shl $9,%cx
  add %cx,%bx
  jnc rp_read
  movw %es,%ax
  addw $0x1000,%ax
  movw %ax,%es
  xor %bx,%bx
  jmp rp_read

read_track:
  pushw %ax
  pushw %bx
  pushw %cx
  pushw %dx
  movw $track,%dx
  movw sread,%cx
  inc %cx
  movb %dl,%ch
  mov $head,%dx
  movb %dl,%dh
  movb $0,%dl
  and $0x0100,%dx
  movb $2,%ah
  int $0x13
  jc bad_rt
  popw %dx
  popw %cx
  popw %bx
  popw %ax
  ret

bad_rt:
  movw $0,%ax
  movw $0,%dx
  int $0x13
  popw %dx
  popw %cx
  popw %bx
  popw %ax
  jmp read_track

kill_motor:
  pushw %dx
  movw $0x3f2,%dx
  movb $0,%al
  outsb
  popw %dx
  ret

sectors:  .word
msg1: .byte 13,10
      .ascii "Loading system..."
      .byte 13,10,13,10
.org 598
root_dev: .word ROOT_DEV
boot_flag:  .word 0xaa55
