#ifndef _Included_org_bblfsh_client_libuast_Libuast_jni_utils
#define _Included_org_bblfsh_client_libuast_Libuast_jni_utils

#include <jni.h>
#include <string>

// Fully qualified Java class names
extern const char CLS_NODE[];
extern const char CLS_CTX_EXT[];
extern const char CLS_CTX[];
extern const char CLS_CTX[];
extern const char CLS_OBJ[];
extern const char CLS_RE[];
extern const char CLS_TO[];

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
extern const char CLS_ITER[];
extern const char CLS_JITER[];

// Method signatures
extern const char METHOD_JNODE_KEY_AT[];
extern const char METHOD_JNODE_VALUE_AT[];
extern const char METHOD_JOBJ_ADD[];
extern const char METHOD_JARR_ADD[];
extern const char METHOD_OBJ_TO_STR[];
extern const char METHOD_RE_INIT[];
extern const char METHOD_RE_INIT_CAUSE[];
extern const char METHOD_ITER_INIT[];
extern const char METHOD_JITER_INIT[];
extern const char METHOD_NODE_INIT[];

// Field signatures
extern const char FIELD_ITER_NODE[];
extern const char FIELD_CTX[];
extern const char FIELD_CTX_EXT[];
// Checks through JNI, if there is a pending excption on the JVM side.
//
// Throws new RuntimeException to the JVM in case there is,
// uses the original one as a cause and the given string as a message.
void checkJvmException(std::string);

// Reads the JVM pointer of the current native thread.
//
// If the thread was not created by JVM - it will be attached to the JVM first.
// Those threads need to be detached later on, in order to avoid memory leaks.
JNIEnv *getJNIEnv();

// Constructs new Java object of a given className and constructor signature.
jobject NewJavaObject(JNIEnv *, const char *, const char *, ...);

// Returns the field ID of the field of the given object.
// The field is specified by its name and signature.
jfieldID FieldID(JNIEnv *, jobject, const char *, const char *);

// Reads the value of an Int field of a given object.
// The field is specified by its name and signature.
jint IntField(JNIEnv *, jobject, const char *, const char *);

// Reads the value of an Object field of a given object.
// The field is specified by its name and signature.
jobject ObjectField(JNIEnv *, jobject, const char *, const char *);

// Returns the method ID for a method of a given class or interface name.
// The method is determined by its name and signature.
jmethodID MethodID(JNIEnv *, const char *, const char *, const char *);

// Calls a method of the given class or interface name that returns an Int.
// The method is determined by its name and signature.
jint IntMethod(JNIEnv *, const char *, const char *, const char *,
               const jobject);

// Calls a method of the given class or interface name that returns an Object.
// The method is determined by its name and signature.
jobject ObjectMethod(JNIEnv *, const char *, const char *, const char *,
                     const jobject, ...);

// Constructs new object the given class name and throws it to JVM.
//
// A fully qualified class name must name a valid Throwable type.
// It does not interfere with the native control flow.
void ThrowByName(JNIEnv *, const char *, const char *);
#endif
