// <COMPONENT>: os-apis
// <FILE-TYPE>: component public header

#ifndef OS_APIS_GCC_COMPAT_H_
#define OS_APIS_GCC_COMPAT_H_

/*
 Emulates GCC's Built-in Function: int __builtin_clz (unsigned int x)
 Returns the number of leading 0-bits in x, starting at the most significant bit position. If x is 0, the result is undefined.
*/
int __builtin_clz(unsigned int x);

/*
 Emulates GCC's Built-in Function: int __builtin_ctz (unsigned int x)
 Returns the number of trailing 0-bits in x, starting at the least significant bit position. If x is 0, the result is undefined.
*/
int __builtin_ctz(unsigned int x);

#endif // file guard
