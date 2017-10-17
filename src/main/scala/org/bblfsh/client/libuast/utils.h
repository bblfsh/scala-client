#ifndef _Included_org_bblfsh_client_libuast_Libuast_utils
#define _Included_org_bblfsh_client_libuast_Libuast_utils

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

// Defined in utils.c
extern const char *SIGN_OBJECT;
extern const char *SIGN_STR;
extern const char *SIGN_SEQ;
extern const char *SIGN_MAP;
extern const char *SIGN_APPLY;
extern const char *SIGN_TOLIST;
extern const char *SIGN_TOIMMLIST;
extern const char *SIGN_LISTINIT;
extern const char *SIGN_APPEND;
extern const char *SIGN_PLUSEQ;
extern const char *CLS_NODE;
extern const char *CLS_ROLE;
extern const char *CLS_SEQ;
extern const char *CLS_MAP;
extern const char *CLS_TUPLE2;
extern const char *CLS_LIST;
extern const char *CLS_MUTLIST;

JNIEnv *getJNIEnv();

jobject *ToObjectPtr(jobject*);

const char *AsNativeStr(jstring);

jint IntMethod(const char *, const char *, const char *, const jobject *);

jobject ObjectMethod(const char *, const char *, const char *, const jobject , ...);

jobject ObjectField(const char *, const jobject *, const char *, const char *);

jobject NewJavaObject(const char *, const char *, ...);

const char *ReadStr(const jobject *, const char *);

int ReadLen(const jobject *, const char *);

#ifdef __cplusplus
}
#endif
#endif
