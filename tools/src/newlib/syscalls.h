
/*
 * System call asm macros
 */
#define syscall0(num)			\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num))
#define syscall1(num, argb)		\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num), "b" (argb))
#define syscall2(num, argb, argc)	\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num), "b" (argb), "c" (argc))
#define syscall3(num, argb, argc, argd)	\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num), "b" (argb), "c" (argc), "d" (argd))
#define syscall4(num, argb, argc, argd, arge)	\
	asm __volatile__(			\
		"int $0x30;" : : "a" (num), "b" (argb), "c" (argc), "d" (argd), "S" (arge))
		
		
#define ssyscall0(num, rettype, name)		\
rettype __##name(void)					\
{									\
	syscall0(num);						\
}									\
rettype name(void) __attribute__((weak, alias("__" #name)))

#define ssyscall1(num, rettype, name, arg1type, arg1)		\
rettype __##name(arg1type arg1)						\
{												\
	syscall1(num, arg1);							\
}												\
rettype name(arg1type arg1) __attribute__((weak, alias("__" #name)))

#define ssyscall2(num, rettype, name, arg1type, arg1, arg2type, arg2)	\
rettype __##name(arg1type arg1, arg2type arg2)				\
{														\
	syscall2(num, arg1, arg2);								\
}														\
rettype name(arg1type arg1, arg2type arg2) __attribute__((weak, alias("__" #name)))

#define ssyscall3(num, rettype, name, arg1type, arg1, arg2type, arg2, arg3type, arg3)	\
rettype __##name(arg1type arg1, arg2type arg2, arg3type arg3)				\
{																\
	syscall3(num, arg1, arg2, arg3);									\
}																\
rettype name(arg1type arg1, arg2type arg2, arg3type arg3)					\
	__attribute__((weak, alias("__" #name)))

#define ssyscall4(num, rettype, name, arg1type, arg1, arg2type, arg2, arg3type, arg3, arg4type, arg4)	\
rettype __##name(arg1type arg1, arg2type arg2, arg3type arg3, arg4type arg4)				\
{																\
	syscall4(num, arg1, arg2, arg3, arg4);									\
}																\
rettype name(arg1type arg1, arg2type arg2, arg3type arg3, arg4type arg4)					\
	__attribute__((weak, alias("__" #name)))

