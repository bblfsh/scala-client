#ifndef _Included_org_bblfsh_client_libuast_Libuast_jni_utils
#define _Included_org_bblfsh_client_libuast_Libuast_jni_utils

#include <jni.h>
#include <string>

// Fully qualified Java class names
extern const char CLS_NODE[];
extern const char CLS_CTX[];
extern const char CLS_OBJ[];
extern const char CLS_RE[];

// Fully qualified class names for Bablefish UAST types
extern const char CLS_JNODE[];
extern const char CLS_JNULL[];
extern const char CLS_JSTR[];
extern const char CLS_JINT[];
extern const char CLS_JFLT[];
extern const char CLS_JBOOL[];
extern const char CLS_JUINT[];
extern const char CLS_JARR[];
extern const char CLS_JOBJ[];

// Method signatures
extern const char METHOD_JNODE_KEY_AT[];
extern const char METHOD_JNODE_VALUE_AT[];
extern const char METHOD_JOBJ_ADD[];
extern const char METHOD_JARR_ADD[];
extern const char METHOD_OBJ_TO_STR[];
extern const char MERHOD_RE_INIT[];
extern const char MERHOD_RE_INIT_CAUSE[];

// Field signatures
extern const char FIELD_NODE[];

// Checks though JNI, if there is a pending excption on JVM side.
//
// Throws new RuntimeExpection to JVM in case there is,
// uses the origial one as a cause and the given string as a message.
void checkJvmException(std::string);

JNIEnv *getJNIEnv();
jobject NewJavaObject(JNIEnv *, const char *, const char *, ...);
jfieldID FieldID(JNIEnv *, jobject, const char *, const char *);
jobject ObjectField(JNIEnv *, jobject, const char *, const char *);

jint IntMethod(JNIEnv *, const char *, const char *, const char *,
               const jobject *);

jobject ObjectMethod(JNIEnv *, const char *, const char *, const char *,
                     const jobject *, ...);

jmethodID MethodID(JNIEnv *, const char *, const char *, const char *);

void ThrowByName(JNIEnv *, const char *, const char *);
#endif
