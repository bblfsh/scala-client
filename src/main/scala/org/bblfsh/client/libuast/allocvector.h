#ifndef _Included_org_bblfsh_client_libuast_Libuast_allocvector
#define _Included_org_bblfsh_client_libuast_Libuast_allocvector

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

// struct to track (malloc)ated objects
typedef struct _AllocVector AllocVector;
extern AllocVector *allocVector;

void trackAllocatedJObject(AllocVector *, jobject *);

void freeAllocVector(AllocVector **);

#ifdef __cplusplus
}
#endif
#endif
