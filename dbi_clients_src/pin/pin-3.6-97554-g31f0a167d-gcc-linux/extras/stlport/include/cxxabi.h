#ifndef _CXXABI_H
#define _CXXABI_H 1

#ifdef __cplusplus
namespace __cxxabiv1
{
extern "C" {
#endif

char*
  __cxa_demangle(const char* __mangled_name, char* __output_buffer,
                 size_t* __length, int* __status);

#ifdef __cplusplus
  }
} // namespace __cxxabiv1

namespace abi = __cxxabiv1;

#endif

#endif // _CXXABI_H
