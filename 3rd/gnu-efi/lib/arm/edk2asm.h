
#define ASM_PFX(x)			x
#define GCC_ASM_EXPORT(x)		\
	.globl		x		; \
	.type		x, %function

