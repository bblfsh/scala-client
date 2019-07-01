#include "jni_utils.h"

extern JavaVM *jvm;  // FIXME(bzz): double-check and document

JNIEnv *getJNIEnv() {
  JNIEnv *pEnv = NULL;

  switch (jvm->GetEnv((void **)&pEnv, JNI_VERSION_1_8)) {
    case JNI_OK:  // Thread is ready to use, nothing to do
      break;

    case JNI_EDETACHED:  // Thread is detached, need to attach
      jvm->AttachCurrentThread((void **)&pEnv, NULL);
      break;
  }

  return pEnv;
}

// Class fully qualified names
const char *CLS_NODE = "org/bblfsh/client/v2/Node";
const char *CLS_CTX = "org/bblfsh/client/v2/Context";

const char *CLS_JNODE = "org/bblfsh/client/v2/JNode";
const char *CLS_JNULL = "org/bblfsh/client/v2/JNull";
const char *CLS_JSTR = "org/bblfsh/client/v2/JString";
const char *CLS_JINT = "org/bblfsh/client/v2/JInt";
const char *CLS_JFLT = "org/bblfsh/client/v2/JFloat";
const char *CLS_JBOL = "org/bblfsh/client/v2/JBool";
const char *CLS_JUINT = "org/bblfsh/client/v2/JUint";
const char *CLS_JARR = "org/bblfsh/client/v2/JArray";
const char *CLS_JOBJ = "org/bblfsh/client/v2/JObject";

const char *METHOD_JNODE_KEY_AT = "(I)Ljava/lang/String;";
const char *METHOD_JNODE_VALUE_AT = "(I)Lorg/bblfsh/client/v2/JNode;";
const char *METHOD_JOBJ_ADD =
    "(Ljava/lang/String;Lorg/bblfsh/client/v2/JNode;)Lscala/collection/"
    "mutable/Buffer;";
const char *METHOD_JARR_ADD =
    "(Lorg/bblfsh/client/v2/JNode;)Lscala/collection/mutable/Buffer;";

jobject NewJavaObject(JNIEnv *env, const char *className, const char *initSign,
                      ...) {
  jclass cls = env->FindClass(className);
  if (env->ExceptionOccurred() || !cls) {
    return nullptr;
  }

  jmethodID initId = env->GetMethodID(cls, "<init>", initSign);
  if (env->ExceptionOccurred() || !initId) {
    return nullptr;
  }

  va_list varargs;
  va_start(varargs, initSign);
  jobject instance = env->NewObjectV(cls, initId, varargs);
  va_end(varargs);
  if (env->ExceptionOccurred() || !instance) {
    return nullptr;
  }

  return instance;
}

jfieldID getField(JNIEnv *env, jobject obj, const char *name) {
  jclass cls = env->GetObjectClass(obj);
  if (env->ExceptionOccurred() || !cls) {
    return nullptr;
  }

  jfieldID jfid = env->GetFieldID(cls, name, "J");
  if (env->ExceptionOccurred() || !jfid) {
    return nullptr;
  }
  return jfid;
}

static jmethodID MethodID(JNIEnv *env, const char *method,
                          const char *signature, const char *className) {
  jclass cls = env->FindClass(className);
  if (env->ExceptionOccurred() || !cls) {
    return nullptr;
  }

  jmethodID mId = env->GetMethodID(cls, method, signature);
  if (env->ExceptionOccurred()) {
    return nullptr;
  }

  return mId;
}

jint IntMethod(JNIEnv *env, const char *method, const char *signature,
               const char *className, const jobject *object) {
  jmethodID mId = MethodID(env, method, signature, className);
  if (env->ExceptionOccurred() || !mId) return 0;
  // TODO(bzz): add better error handling
  //  ExceptionOccurred()
  //  ExceptionDescribe()
  //  ExceptionClear()
  //  ExceptionCheck()
  //  ThrowNew("failed to call $className.$method") to JVM

  jint res = env->CallIntMethod(*object, mId);
  if (env->ExceptionOccurred()) return 0;

  return res;
}

jobject ObjectMethod(JNIEnv *env, const char *method, const char *signature,
                     const char *className, const jobject *object, ...) {
  jmethodID mId = MethodID(env, method, signature, className);
  if (env->ExceptionOccurred() || !mId) return nullptr;

  va_list varargs;
  va_start(varargs, object);
  jobject res = env->CallObjectMethodV(*object, mId, varargs);
  va_end(varargs);
  if (env->ExceptionOccurred() || !res) return nullptr;

  return res;
}
