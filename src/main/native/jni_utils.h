#ifndef _Included_org_bblfsh_client_libuast_Libuast_jni_utils
#define _Included_org_bblfsh_client_libuast_Libuast_jni_utils

#include <jni.h>
#include <string>

extern const char *CLS_NODE;
extern const char *CLS_CTX;
extern const char *CLS_OBJ;
extern const char *CLS_RE;

extern const char *CLS_JNODE;
extern const char *CLS_JNULL;
extern const char *CLS_JSTR;
extern const char *CLS_JINT;
extern const char *CLS_JFLT;
extern const char *CLS_JBOL;
extern const char *CLS_JUINT;
extern const char *CLS_JARR;
extern const char *CLS_JOBJ;

extern const char *METHOD_JNODE_KEY_AT;
extern const char *METHOD_JNODE_VALUE_AT;
extern const char *METHOD_JOBJ_ADD;
extern const char *METHOD_JARR_ADD;

extern const char *METHOD_OBJ_TO_STR;

void checkJvmException(std::string);

JNIEnv *getJNIEnv();
jobject NewJavaObject(JNIEnv *, const char *, const char *, ...);
jfieldID getField(JNIEnv *env, jobject obj, const char *name);

jint IntMethod(JNIEnv *, const char *, const char *, const char *,
               const jobject *);

jobject ObjectMethod(JNIEnv *, const char *, const char *, const char *,
                     const jobject *, ...);

#endif