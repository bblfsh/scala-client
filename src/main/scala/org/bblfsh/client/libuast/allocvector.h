#ifndef _Included_org_bblfsh_client_libuast_Libuast_allocvector
#define _Included_org_bblfsh_client_libuast_Libuast_allocvector

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

// struct to track (malloc)ated objects
typedef struct {
  jobject **vector;
  size_t used;
  size_t size;
} AllocVector;


void initAllocVector(AllocVector *l);

void trackAllocatedJObject(AllocVector *l, jobject *obj);

void freeAllocVector(AllocVector *l);

#ifdef __cplusplus
}
#endif
#endif
