#ifndef _Included_org_bblfsh_client_libuast_Libuast_jni_utils
#define _Included_org_bblfsh_client_libuast_Libuast_jni_utils

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

// Defined in utils.c
extern const char *TYPE_STR;
extern const char *TYPE_OPTION;
extern const char *TYPE_SEQ;
extern const char *TYPE_MAP;

extern const char *METHOD_SEQ_APPLY;
extern const char *METHOD_MAP_APPLY;
extern const char *METHOD_MUTLIST_TOLIST;
extern const char *METHOD_MUTLIST_TOIMMLIST;
extern const char *METHOD_LIST_INIT;
extern const char *METHOD_LIST_PLUSEQ;
extern const char *METHOD_OPTION_GET;
extern const char *METHOD_SEQ_SORTED;
extern const char *METHOD_MAP_KEYS;

extern const char *CLS_NODE;
extern const char *CLS_POSITION;
extern const char *CLS_ROLE;
extern const char *CLS_OPTION;
extern const char *CLS_SEQ;
extern const char *CLS_MAP;
extern const char *CLS_LIST;
extern const char *CLS_MUTLIST;
extern const char *CLS_ITERABLE;

JNIEnv *getJNIEnv();

jobject *ToObjectPtr(jobject*);

const char *AsNativeStr(jstring);

jint IntMethod(JNIEnv *, const char *, const char *, const char *, const jobject *);

jboolean BooleanMethod(JNIEnv *, const char *, const char *, const char *, const jobject *);

jobject ObjectMethod(JNIEnv *, const char *, const char *, const char *, const jobject *, ...);

jobject ObjectField(JNIEnv *, const char *, const jobject *, const char *, const char *);

jint IntField(JNIEnv *, const char *, const jobject *, const char *);

jobject NewJavaObject(JNIEnv *, const char *, const char *, ...);

const char *ReadStr(const jobject *, const char *);

int ReadLen(const jobject *, const char *);

#ifdef __cplusplus
}
#endif
#endif
