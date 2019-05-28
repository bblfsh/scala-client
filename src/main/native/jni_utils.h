#ifndef _Included_org_bblfsh_client_libuast_Libuast_jni_utils
#define _Included_org_bblfsh_client_libuast_Libuast_jni_utils

#include <jni.h>

extern const char *CLS_NODE;
extern const char *CLS_CTX;

JNIEnv *getJNIEnv();
jobject NewJavaObject(JNIEnv *, const char *, const char *, ...);
jfieldID getField(JNIEnv *env, jobject obj, const char *name);

#endif