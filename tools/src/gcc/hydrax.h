/* Useful if you wish to make target-specific gcc changes */
#undef TARGET_HYDRAX
#define TARGET_HYDRAX 1
 
/* Don't automatically add extern "C" { } around header files. */
#undef  NO_IMPLICIT_EXTERN_C
#define NO_IMPLICIT_EXTERN_C 1
  
/* Additional predefined macros. */
#undef TARGET_OS_CPP_BUILTINS
#define TARGET_OS_CPP_BUILTINS()      		\
do 						\
{                                		\
	builtin_define ("__myos__");      	\
	builtin_define ("__unix__");      	\
	builtin_assert ("system=hydrax");   	\
	builtin_assert ("system=unix");   	\
	builtin_assert ("system=posix");   	\
} 						\
while(0);
