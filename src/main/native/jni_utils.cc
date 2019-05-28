#include "jni_utils.h"

// Class fully qualified names
const char *CLS_NODE = "org/bblfsh/client/v2/Node";
const char *CLS_CTX = "org/bblfsh/client/v2/Context";

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

jobject NewJavaObject(JNIEnv *env, const char *className, const char *initSign,
                      ...) {
  jclass cls = env->FindClass(className);
  if (env->ExceptionOccurred() || !cls) {
    return NULL;
  }

  jmethodID initId = env->GetMethodID(cls, "<init>", initSign);
  if (env->ExceptionOccurred() || !initId) {
    return NULL;
  }

  va_list varargs;
  va_start(varargs, initSign);
  jobject instance = env->NewObjectV(cls, initId, varargs);
  va_end(varargs);
  if (env->ExceptionOccurred() || !instance) {
    return NULL;
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
