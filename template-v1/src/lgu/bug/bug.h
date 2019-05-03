#ifndef SRC_LGU_BUG_BUG_H_
#define SRC_LGU_BUG_BUG_H_

#include <stdio.h>
#include <assert.h>

#define BUG_ON_ENABLE (1)

#if (BUG_ON_ENABLE)
#define BUG_ON(_condition) do { assert(!(_condition)); } while (0)
#define WARN_ON(_condition) do { if ((_condition)) { fprintf(stderr, " * WARNING: At %s:%d, %s\n", __FUNCTION__, __LINE__, #_condition); } } while (0)
#else
#define BUG_ON(_condition) do { int __useless__ = (_condition); } while (0)
#define WARN_ON(_condition) do { int __useless__ = (_condition); } while (0)
#endif

#define BUG() do { int *p = NULL; fprintf(stderr, " * BUG: %s:%d\n", __FUNCTION__, __LINE__); fflush(stderr); *p = 999; } while (0)

#endif /* SRC_LGU_BUG_BUG_H_ */
