#ifdef __cplusplus
extern "C" {
#endif

#include "utils.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// XXX remove
#include <stdio.h>

// Type signatures; to get the signature of all methods from a class do:
// javap -s -p SomeClass.class
// To create a Java version of this module from the same codebase, #ifdefs should
// be sprinkled here to get the equivalent Java types.
const char *SIGN_OBJECT = "Ljava/lang/Object;";
const char *SIGN_STR = "Ljava/lang/String;";
const char *SIGN_OPTION = "Lscala/Option;";
const char *SIGN_SEQ = "Lscala/collection/Seq;";
const char *SIGN_MAP = "Lscala/collection/immutable/Map;";

// Method signatures. Same as above: use javap to get them from a .class
const char *SIGN_APPLY = "(I)Ljava/lang/Object;";
const char *SIGN_TOLIST = "()Lscala/collection/immutable/List;";
const char *SIGN_TOIMMLIST = "()Lscala/collection/immutable/List;";
const char *SIGN_LISTINIT = "()V";
const char *SIGN_APPEND = "(Ljava/lang/Object;)V";
const char *SIGN_PLUSEQ = "(Ljava/lang/Object;)Lscala/collection/mutable/MutableList;";
const char *SIGN_OPTION_GET = "()Ljava/lang/Object;";
const char *SIGN_SORTED = "(Lscala/math/Ordering;)Ljava/lang/Object;";
const char *SIGN_KEYS = "()Lscala/collection/GenIterable;";

// Class fully qualified names
const char *CLS_NODE = "gopkg/in/bblfsh/sdk/v1/uast/generated/Node";
const char *CLS_POSITION = "gopkg/in/bblfsh/sdk/v1/uast/generated/Position";
const char *CLS_ROLE = "gopkg/in/bblfsh/sdk/v1/uast/generated/Role";
const char *CLS_OPTION = "scala/Option";
const char *CLS_SEQ = "scala/collection/Seq";
const char *CLS_MAP = "scala/collection/Map";
const char *CLS_TUPLE2 = "scala/Tuple2";
const char *CLS_LIST = "scala/collection/immutable/List";
const char *CLS_MUTLIST = "scala/collection/mutable/MutableList";
const char *CLS_ITERABLE = "scala/collection/GenIterable";

extern JavaVM *jvm;

JNIEnv *getJNIEnv() {
  JNIEnv *pEnv = NULL;

  switch ((*jvm)->GetEnv(jvm, (void **)&pEnv, JNI_VERSION_1_8))
  {
    case JNI_OK:
      // Thread is ready to use, nothing to do
      break;

    case JNI_EDETACHED:
      // Thread is detached, need to attach
      (*jvm)->AttachCurrentThread(jvm, (void **)&pEnv, NULL);
      break;
  }

  return pEnv;
}

const char *AsNativeStr(jstring jstr) {
  if (!jstr)
    return NULL;

  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  const char *tmp = (*env)->GetStringUTFChars(env, jstr, 0);
  if ((*env)->ExceptionOccurred(env) || !tmp)
    return NULL;

  // str must be copied to deref the java string before return
  const char *cstr = strdup(tmp);

  (*env)->ReleaseStringUTFChars(env, jstr, tmp);
  if ((*env)->ExceptionOccurred(env))
    return NULL;

  return cstr;
}

jobject *ToObjectPtr(jobject *object) {
  jobject *copy = malloc(sizeof(jobject));
  memcpy(copy, object, sizeof(jobject));
  return copy;
}

jint IntMethod(const char *method, const char *signature, const char *className,
              const jobject *object) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return 0;

  jclass cls = (*env)->FindClass(env, className);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return 0;

  jmethodID mId = (*env)->GetMethodID(env, cls, method, signature);
  if ((*env)->ExceptionOccurred(env))
    return 0;

  jint res = (*env)->CallIntMethod(env, *object, mId);
  if ((*env)->ExceptionOccurred(env))
    return 0;

  return res;
}

jboolean BooleanMethod(const char *method, const char *signature, const char *className,
              const jobject *object) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return false;

  jclass cls = (*env)->FindClass(env, className);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return false;

  jmethodID mId = (*env)->GetMethodID(env, cls, method, signature);
  if ((*env)->ExceptionOccurred(env))
    return false;

  jboolean res = (*env)->CallBooleanMethod(env, *object, mId);
  if ((*env)->ExceptionOccurred(env))
    return false;

  return res;
}

jobject ObjectMethod(const char *method, const char *signature, const char *typeName,
                     const jobject object, ...) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jclass cls = (*env)->FindClass(env, typeName);
  printf("XXX a\n");
  if ((*env)->ExceptionOccurred(env) || !cls) {
    printf("XXX b\n");
    return NULL;
  }
  printf("XXX c\n");

  printf("XXX d\n");
  jmethodID mId = (*env)->GetMethodID(env, cls, method, signature);
  if ((*env)->ExceptionOccurred(env) || !mId)
    return NULL;
  printf("XXX e\n");

  va_list varargs;
  va_start(varargs, object);
  printf("XXX f\n");
  jobject res = (*env)->CallObjectMethodV(env, object, mId, varargs);
  va_end(varargs);
  if ((*env)->ExceptionOccurred(env) || !res)
    return NULL;
  printf("XXX g\n");

  return res;
}

jobject ObjectField(const char *typeName, const jobject *obj, const char *field,
                    const char *signature) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jclass cls = (*env)->FindClass(env, typeName);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return NULL;

  // Note: printing the type from Scala to find the type needed for GetFieldID
  // third argument using getClass.getName sometimes return objects different
  // from the ones needed for the signature. To find the right type to use do
  // this from Scala: (instance).getClass.getDeclaredField("fieldName")
  jfieldID valueId = (*env)->GetFieldID(env, cls, field, signature);
  if ((*env)->ExceptionOccurred(env) || !valueId)
    return NULL;

  jobject value = (*env)->GetObjectField(env, *obj, valueId);
  if ((*env)->ExceptionOccurred(env) || !value)
    return NULL;

  return value;
}

jint IntField(const char *typeName, const jobject *obj, const char *field) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return 0;

  jclass cls = (*env)->FindClass(env, typeName);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return 0;

  jfieldID valueId = (*env)->GetFieldID(env, cls, field, "I");
  if ((*env)->ExceptionOccurred(env) || !valueId)
    return 0;

  jint value = (*env)->GetIntField(env, *obj, valueId);
  if ((*env)->ExceptionOccurred(env) || !value)
    return 0;

  return value;
}

jobject NewJavaObject(const char *className, const char *initSign, ...) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jclass cls = (*env)->FindClass(env, className);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return NULL;

  jmethodID initId = (*env)->GetMethodID(env, cls, "<init>", initSign);
  if ((*env)->ExceptionOccurred(env) || !initId)
    return NULL;

  va_list varargs;
  va_start(varargs, initSign);
  jobject instance = (*env)->NewObjectV(env, cls, initId, varargs);
  va_end(varargs);
  if ((*env)->ExceptionOccurred(env) || !instance)
    return NULL;

  return instance;
}

const char *ReadStr(const jobject *node, const char *property) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return NULL;

  jclass cls = (*env)->FindClass(env, CLS_NODE);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return NULL;

  jstring jvstr = (jstring)ObjectField(CLS_NODE, node, property, SIGN_STR);
  if ((*env)->ExceptionOccurred(env) || !jvstr)
    return NULL;

  return AsNativeStr(jvstr);
}

int ReadLen(const jobject *node, const char *property) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return 0;

  jclass cls = (*env)->FindClass(env, CLS_NODE);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return 0;

  jobject childSeq = ObjectField(CLS_NODE, node, property, SIGN_SEQ);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return 0;

  return (int)IntMethod("length", "()I", CLS_SEQ, &childSeq);
}

#ifdef __cplusplus
}
#endif
