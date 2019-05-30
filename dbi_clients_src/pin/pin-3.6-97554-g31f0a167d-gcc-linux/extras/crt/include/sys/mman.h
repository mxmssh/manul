#ifndef _SYS_MMAN_H_
#define _SYS_MMAN_H_

#ifdef TARGET_MAC
#include <sys/mman_mac.h>
#else
#include <sys/mman_nonmac.h>
#endif


#endif /* _SYS_MMAN_H_ */
