#include "jni_utils.h"
#include <string>

// TODO(bzz): double-check and document. Suggestion and more context at
// https://github.com/bblfsh/scala-client/pull/84#discussion_r288347756
extern JavaVM *jvm;

JNIEnv *getJNIEnv() {
  JNIEnv *pEnv = NULL;

  switch (jvm->GetEnv((void **)&pEnv, JNI_VERSION_1_8)) {
    case JNI_OK:  // Thread is ready to use, nothing to do
      break;

    case JNI_EDETACHED:  // Thread is detached, need to attach
      jvm->AttachCurrentThread((void **)&pEnv, NULL);
      // TODO: this is a memory leak (at least)
      //  - jvm->DetachCurrentThread() is never called
      //  - local references are _never_ deleted :scream:
      //    https://developer.android.com/training/articles/perf-jni#local-and-global-references
      break;
  }

  return pEnv;
}

// Class fully qualified names
const char CLS_NODE[] = "org/bblfsh/client/v2/NodeExt";
const char CLS_CTX[] = "org/bblfsh/client/v2/ContextExt";
const char CLS_OBJ[] = "java/lang/Object";
const char CLS_RE[] = "java/lang/RuntimeException";
const char CLS_JNODE[] = "org/bblfsh/client/v2/JNode";
const char CLS_JNULL[] = "org/bblfsh/client/v2/JNull";
const char CLS_JSTR[] = "org/bblfsh/client/v2/JString";
const char CLS_JINT[] = "org/bblfsh/client/v2/JInt";
const char CLS_JFLT[] = "org/bblfsh/client/v2/JFloat";
const char CLS_JBOOL[] = "org/bblfsh/client/v2/JBool";
const char CLS_JUINT[] = "org/bblfsh/client/v2/JUint";
const char CLS_JARR[] = "org/bblfsh/client/v2/JArray";
const char CLS_JOBJ[] = "org/bblfsh/client/v2/JObject";

// Method signatures
const char METHOD_JNODE_KEY_AT[] = "(I)Ljava/lang/String;";
const char METHOD_JNODE_VALUE_AT[] = "(I)Lorg/bblfsh/client/v2/JNode;";
const char METHOD_JOBJ_ADD[] =
    "(Ljava/lang/String;Lorg/bblfsh/client/v2/JNode;)Lscala/collection/"
    "mutable/Buffer;";
const char METHOD_JARR_ADD[] =
    "(Lorg/bblfsh/client/v2/JNode;)Lscala/collection/mutable/Buffer;";

const char METHOD_OBJ_TO_STR[] = "()Ljava/lang/String;";

// TODO(bzz): cache classes&methods in JNI_OnLoad should speed this up
void checkJvmException(std::string msg) {
  JNIEnv *env = getJNIEnv();
  auto err = env->ExceptionOccurred();
  if (err) {
    env->ExceptionClear();

    auto exceptionCls = env->FindClass(CLS_RE);
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      env->Throw(env->ExceptionOccurred());
      return;
    }

    jclass cls = env->FindClass(CLS_OBJ);
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      env->ThrowNew(
          exceptionCls,
          msg.append(" - failed to find class ").append(CLS_OBJ).data());
      return;
    }

    jmethodID toString = env->GetMethodID(cls, "toString", METHOD_OBJ_TO_STR);
    if (env->ExceptionCheck()) {
      env->ExceptionClear();
      env->ThrowNew(exceptionCls,
                    msg.append(" - failed to find method toString").data());
      return;
    }

    jstring s = (jstring)env->CallObjectMethod(err, toString);
    if (env->ExceptionCheck() || !s) {
      env->ThrowNew(exceptionCls,
                    msg.append(" - failed co call method toString").data());
      return;
    }

    const char *utf = env->GetStringUTFChars(s, 0);
    env->ReleaseStringUTFChars(s, utf);

    // new RuntimeException(msg.data(), err)
    jmethodID initId = env->GetMethodID(
        cls, "<init>", "(Ljava/lang/String;Ljava/lang/Throwable;)V");
    jthrowable exception = (jthrowable)env->NewObject(
        exceptionCls, initId, msg.append(": ").append(utf).data(), err);

    env->Throw(exception);
  }
}

jobject NewJavaObject(JNIEnv *env, const char *className, const char *initSign,
                      ...) {
  jclass cls = env->FindClass(className);
  checkJvmException(std::string("failed to find a class ").append(className));

  jmethodID initId = env->GetMethodID(cls, "<init>", initSign);
  checkJvmException(std::string("failed to call a constructor with signature ")
                        .append(initSign)
                        .append(" for the class name ")
                        .append(className));

  va_list varargs;
  va_start(varargs, initSign);
  jobject instance = env->NewObjectV(cls, initId, varargs);
  va_end(varargs);
  checkJvmException(
      std::string("failed get varargs for constructor of ").append(className));

  return instance;
}

jfieldID getField(JNIEnv *env, jobject obj, const char *name) {
  jclass cls = env->GetObjectClass(obj);
  checkJvmException("failed get the class of an object");

  jfieldID jfid = env->GetFieldID(cls, name, "J");
  checkJvmException(std::string("failed get a field ").append(name));

  return jfid;
}

jmethodID MethodID(JNIEnv *env, const char *method, const char *signature,
                   const char *className) {
  jclass cls = env->FindClass(className);
  checkJvmException(std::string("failed to find a class ").append(className));

  jmethodID mId = env->GetMethodID(cls, method, signature);
  checkJvmException(std::string("failed to get method ")
                        .append(className)
                        .append(".")
                        .append(method));

  return mId;
}

jint IntMethod(JNIEnv *env, const char *method, const char *signature,
               const char *className, const jobject *object) {
  jmethodID mId = MethodID(env, method, signature, className);
  checkJvmException(std::string("failed to get method ")
                        .append(className)
                        .append(".")
                        .append(method));

  jint res = env->CallIntMethod(*object, mId);
  checkJvmException(std::string("failed to call method ")
                        .append(className)
                        .append(".")
                        .append(method)
                        .append(" using signature ")
                        .append(signature));

  return res;
}

jobject ObjectMethod(JNIEnv *env, const char *method, const char *signature,
                     const char *className, const jobject *object, ...) {
  jmethodID mId = MethodID(env, method, signature, className);
  checkJvmException(std::string("failed to get method ")
                        .append(className)
                        .append(".")
                        .append(method));

  va_list varargs;
  va_start(varargs, object);
  jobject res = env->CallObjectMethodV(*object, mId, varargs);
  va_end(varargs);
  checkJvmException(std::string("failed to get varargs for ")
                        .append(className)
                        .append(".")
                        .append(method));

  return res;
}
