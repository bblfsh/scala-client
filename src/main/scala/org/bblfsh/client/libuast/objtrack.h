#ifndef _Included_org_bblfsh_client_libuast_Libuast_allocvector
#define _Included_org_bblfsh_client_libuast_Libuast_allocvector

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

void trackObject(jobject *);

void freeObjects();

#ifdef __cplusplus
}
#endif
#endif
