#ifdef __cplusplus
extern "C" {
#endif

#include "jni_utils.h"
#include "objtrack.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Type signatures; to get the signature of all methods from a class do:
// javap -s -p SomeClass.class
// To create a Java version of this module from the same codebase, #ifdefs should
// be sprinkled here to get the equivalent Java types.
const char *TYPE_STR = "Ljava/lang/String;";
const char *TYPE_OPTION = "Lscala/Option;";
const char *TYPE_SEQ = "Lscala/collection/Seq;";
const char *TYPE_MAP = "Lscala/collection/immutable/Map;";

// Method signatures. Same as above: use javap to get them from a .class
const char *METHOD_SEQ_APPLY = "(I)Ljava/lang/Object;";
const char *METHOD_MAP_APPLY = "(Ljava/lang/Object;)Ljava/lang/Object;";
const char *METHOD_MUTLIST_TOLIST = "()Lscala/collection/immutable/List;";
const char *METHOD_MUTLIST_TOIMMLIST = "()Lscala/collection/immutable/List;";
const char *METHOD_LIST_INIT = "()V";
const char *METHOD_LIST_PLUSEQ = "(Ljava/lang/Object;)Lscala/collection/mutable/MutableList;";
const char *METHOD_OPTION_GET = "()Ljava/lang/Object;";
const char *METHOD_SEQ_SORTED = "(Lscala/math/Ordering;)Ljava/lang/Object;";
const char *METHOD_MAP_KEYS = "()Lscala/collection/GenIterable;";

// Class fully qualified names
const char *CLS_NODE = "gopkg/in/bblfsh/sdk/v1/uast/generated/Node";
const char *CLS_POSITION = "gopkg/in/bblfsh/sdk/v1/uast/generated/Position";
const char *CLS_ROLE = "gopkg/in/bblfsh/sdk/v1/uast/generated/Role";
const char *CLS_OPTION = "scala/Option";
const char *CLS_SEQ = "scala/collection/Seq";
const char *CLS_MAP = "scala/collection/Map";
const char *CLS_LIST = "scala/collection/immutable/List";
const char *CLS_MUTLIST = "scala/collection/mutable/MutableList";
const char *CLS_ITERABLE = "scala/collection/GenIterable";
// XXX
const char *CLS_LONG = "java/lang/Long";

extern JavaVM *jvm;

//// JNI helpers
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
  trackObject((void *)cstr);

  (*env)->ReleaseStringUTFChars(env, jstr, tmp);
  if ((*env)->ExceptionOccurred(env))
    return NULL;

  return cstr;
}

jobject *ToObjectPtr(jobject *object) {
  jobject *copy = malloc(sizeof(jobject));
  memcpy(copy, object, sizeof(jobject));
  trackObject((void *)copy);
  return copy;
}

static jmethodID MethodID(JNIEnv *env, const char *method, const char *signature,
                   const char *className, const jobject *object) {
  jclass cls = (*env)->FindClass(env, className);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return NULL;

  jmethodID mId = (*env)->GetMethodID(env, cls, method, signature);
  if ((*env)->ExceptionOccurred(env))
    return NULL;

  return mId;
}

static jfieldID FieldID(JNIEnv *env, const char *className, const char *field,
                        const char *typeSignature) {
  jclass cls = (*env)->FindClass(env, className);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return NULL;

  // Note: printing the type from Scala to find the type needed for GetFieldID
  // third argument using getClass.getName sometimes return objects different
  // from the ones needed for the signature. To find the right type to use do
  // this from Scala: (instance).getClass.getDeclaredField("fieldName")
  jfieldID fId = (*env)->GetFieldID(env, cls, field, typeSignature);
  if ((*env)->ExceptionOccurred(env) || !fId)
    return NULL;

  return fId;
}

jint IntMethod(JNIEnv *env, const char *method, const char *signature, const char *className,
               const jobject *object) {
  jmethodID mId = MethodID(env, method, signature, className, object);
  if ((*env)->ExceptionOccurred(env) || !mId)
    return 0;

  printf("XXX IntMethod CRASH HERE, object ptr: %p, mId: %p\n", object, mId);
  // XXX the object doesnt have the same addr here as in the previous calls
  jint res = (*env)->CallIntMethod(env, *object, mId);
  if ((*env)->ExceptionOccurred(env)) {
    return 0;
  }

  return res;
}

jlong LongMethod(JNIEnv *env, const char *method, const char *signature, const char *className,
               const jobject *object) {
  jmethodID mId = MethodID(env, method, signature, className, object);
  if ((*env)->ExceptionOccurred(env) || !mId)
    return 0;

  jlong res = (*env)->CallLongMethod(env, *object, mId);
  if ((*env)->ExceptionOccurred(env))
    return 0;

  return res;
}

jboolean BooleanMethod(JNIEnv *env, const char *method, const char *signature,
                       const char *className, const jobject *object) {
  jmethodID mId = MethodID(env, method, signature, className, object);
  if ((*env)->ExceptionOccurred(env) || !mId)
    return false;

  jboolean res = (*env)->CallBooleanMethod(env, *object, mId);
  if ((*env)->ExceptionOccurred(env))
    return false;

  return res;
}

jobject ObjectMethod(JNIEnv *env, const char *method, const char *signature,
                     const char *className, const jobject *object, ...) {
  jmethodID mId = MethodID(env, method, signature, className, object);
  if ((*env)->ExceptionOccurred(env) || !mId)
    return false;

  va_list varargs;
  va_start(varargs, object);
  jobject res = (*env)->CallObjectMethodV(env, *object, mId, varargs);
  va_end(varargs);
  if ((*env)->ExceptionOccurred(env) || !res)
    return NULL;

  return res;
}

jobject ObjectField(JNIEnv *env, const char *className, const jobject *obj,
                    const char *field, const char *typeSignature) {
  jfieldID valueId = FieldID(env, className, field, typeSignature);
  if ((*env)->ExceptionOccurred(env) || !valueId)
    return NULL;

  jobject value = (*env)->GetObjectField(env, *obj, valueId);
  if ((*env)->ExceptionOccurred(env) || !value)
    return NULL;

  return value;
}

jint IntField(JNIEnv *env, const char *className, const jobject *obj, const char *field) {
  jfieldID valueId = FieldID(env, className, field, "I");
  if ((*env)->ExceptionOccurred(env) || !valueId)
    return 0;

  jint value = (*env)->GetIntField(env, *obj, valueId);
  if ((*env)->ExceptionOccurred(env) || !value)
    return 0;

  return value;
}

jobject NewJavaObject(JNIEnv *env, const char *className, const char *initSign, ...) {
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

  jstring jvstr = (jstring)ObjectField(env, CLS_NODE, node, property, TYPE_STR);
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

  printf("XXX jni_utils: CRASH HERE, node ptr: %p\n", node);
  jobject childSeq = ObjectField(env, CLS_NODE, node, property, TYPE_SEQ);
  if ((*env)->ExceptionOccurred(env) || !childSeq) {
    return 0;
  }

  return (int)IntMethod(env, "length", "()I", CLS_SEQ, &childSeq);
}

void ThrowException(const char* message) {
  JNIEnv *env = getJNIEnv();
  if (!env)
    return;

  jclass jcls = (*env)->FindClass(env, "java/lang/Exception");
  if ((*env)->ExceptionCheck(env) == JNI_TRUE)
    return;

  (*env)->ThrowNew(env, jcls, message);
}

#ifdef __cplusplus
}
#endif
