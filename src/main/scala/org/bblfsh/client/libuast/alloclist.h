#ifndef _Included_org_bblfsh_client_libuast_Libuast_alloclist
#define _Included_org_bblfsh_client_libuast_Libuast_alloclist

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

// struct to track (malloc)ated objects
typedef struct {
  jobject **list;
  size_t used;
  size_t size;
} AllocList;


void initAllocList(AllocList *l, size_t initialSize);

void trackAllocatedJObject(AllocList *l, jobject *obj);

void freeAllocList(AllocList *l);

#ifdef __cplusplus
}
#endif
#endif
