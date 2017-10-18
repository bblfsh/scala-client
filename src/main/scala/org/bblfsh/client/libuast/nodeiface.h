#ifndef _Included_org_bblfsh_client_libuast_Libuast_nodeiface
#define _Included_org_bblfsh_client_libuast_Libuast_nodeiface

#ifdef __cplusplus
extern "C" {
#endif

#include "utils.h"

#include "uast.h"

// XXX remove
#include <stdint.h>

Uast *CreateUast();

// XXX remove
bool _HasPosition(const void *, const char *);
const char *PropertyKeyAt(const void *, int);
const char *PropertyValueAt(const void *, int);
uint32_t StartOffset(const void *);
uint32_t StartLine(const void *);
uint32_t StartCol(const void *);
uint32_t EndOffset(const void *);
uint32_t EndLine(const void *);
uint32_t EndCol(const void *);

#ifdef __cplusplus
}
#endif
#endif
