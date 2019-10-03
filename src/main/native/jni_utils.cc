#include "jni_utils.h"

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
      // FIXME(bzz): this is a memory leak (at least)
      //  - jvm->DetachCurrentThread() is never called
      //  - local references are _never_ deleted :scream:
      //    https://developer.android.com/training/articles/perf-jni#local-and-global-references
      break;
  }

  return pEnv;
}

// Class fully qualified names
const char CLS_NODE[] = "org/bblfsh/client/v2/NodeExt";
const char CLS_CTX_EXT[] = "org/bblfsh/client/v2/ContextExt";
const char CLS_CTX[] = "org/bblfsh/client/v2/Context";
const char CLS_TO[] = "org/bblfsh/client/v2/libuast/Libuast$TreeOrder";
const char CLS_ENCS[] = "org/bblfsh/client/v2/libuast/Libuast$UastFormat";
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
const char CLS_ITER[] = "org/bblfsh/client/v2/libuast/Libuast$UastIterExt";
const char CLS_JITER[] = "org/bblfsh/client/v2/libuast/Libuast$UastIter";

// Method signatures
const char METHOD_JNODE_KEY_AT[] = "(I)Ljava/lang/String;";
const char METHOD_JNODE_VALUE_AT[] = "(I)Lorg/bblfsh/client/v2/JNode;";
const char METHOD_JOBJ_ADD[] =
    "(Ljava/lang/String;Lorg/bblfsh/client/v2/JNode;)Lscala/collection/"
    "mutable/Buffer;";
const char METHOD_JARR_ADD[] =
    "(Lorg/bblfsh/client/v2/JNode;)Lscala/collection/mutable/Buffer;";

const char METHOD_OBJ_TO_STR[] = "()Ljava/lang/String;";

const char METHOD_RE_INIT[] = "(Ljava/lang/String;)V";
const char METHOD_RE_INIT_CAUSE[] =
    "(Ljava/lang/String;Ljava/lang/Throwable;)V";

const char METHOD_ITER_INIT[] = "(Lorg/bblfsh/client/v2/NodeExt;IJLorg/bblfsh/client/v2/ContextExt;)V";
const char METHOD_JITER_INIT[] = "(Lorg/bblfsh/client/v2/JNode;IJLorg/bblfsh/client/v2/Context;)V";

const char METHOD_NODE_INIT[] = "(Lorg/bblfsh/client/v2/ContextExt;J)V";

// Field signatures
const char FIELD_ITER_NODE[] = "Ljava/lang/Object;";
const char FIELD_CTX[] = "Lorg/bblfsh/client/v2/Context;";
const char FIELD_CTX_EXT[] = "Lorg/bblfsh/client/v2/ContextExt;";


void clearClassCache(JNIEnv *env) {
  for (auto cached : classCache) {
    env->DeleteGlobalRef(cached.second);
  }
}


jclass findClass(JNIEnv *env, const char *className) {
  jclass result = classCache[className];

  if (!result) {
    jclass localClassRef = env->FindClass(className);

    if (!localClassRef) {
      checkJvmException(std::string("failed to find a class ").append(className));
    }
    
    result = (jclass) env->NewGlobalRef(localClassRef);
    classCache[className] = result;
    env->DeleteLocalRef(localClassRef);
  }

  return result;
}


void checkJvmException(std::string msg) {
  JNIEnv *env = getJNIEnv();
  auto err = env->ExceptionOccurred();
  if (err) {
    env->ExceptionClear();

    jstring s = (jstring)env->CallObjectMethod(err, exceptToString);
    if (env->ExceptionCheck() || !s) {
      env->ExceptionClear();
      env->ThrowNew(exceptionCls,
                    msg.append(" - failed co call method toString").data());
      return;
    }

    const char *utf = env->GetStringUTFChars(s, 0);
    jstring jmsg = env->NewStringUTF(msg.append(": ").append(utf).c_str());
    env->ReleaseStringUTFChars(s, utf);

    // new RuntimeException(jmsg, err)
    jmethodID initId = MethodID(env, "<init>", METHOD_RE_INIT_CAUSE, CLS_RE);

    if (env->ExceptionCheck() || !initId) {
      env->ExceptionClear();
      env->ThrowNew(exceptionCls,
                    msg.append(" - failed to get method id for signature ")
                        .append(METHOD_RE_INIT_CAUSE)
                        .c_str());
      return;
    }

    jthrowable exception =
        (jthrowable)env->NewObject(exceptionCls, initId, jmsg, err);
    if (env->ExceptionCheck() || !exception) {
      env->ExceptionClear();
      env->ThrowNew(exceptionCls,
                    msg.append(" - failed to create a new instance of ")
                        .append(CLS_RE)
                        .append(METHOD_RE_INIT_CAUSE)
                        .c_str());
      return;
    }

    env->Throw(exception);
  }
}

jobject NewJavaObject(JNIEnv *env, const char *className, const char *initSign,
                      ...) {
  jclass cls = findClass(env, className);
  checkJvmException(std::string("failed to find a class ").append(className));

  jmethodID initId = MethodID(env, "<init>", initSign, className);
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

jfieldID FieldID(JNIEnv *env, jobject obj, const char *field,
                 const char *typeSignature) {
  jclass cls = env->GetObjectClass(obj);
  checkJvmException("failed get the class of an object");

  // Note: printing the type from Scala to find the type needed for GetFieldID
  // third argument using getClass.getName sometimes return objects different
  // from the ones needed for the signature. To find the right type to use do
  // this from Scala: (instance).getClass.getDeclaredField("fieldName")
  jfieldID fId = env->GetFieldID(cls, field, typeSignature);
  checkJvmException(std::string("failed get a field ID '")
                        .append(field)
                        .append("' for type signature '")
                        .append(typeSignature)
                        .append("'"));

  return fId;
}

jobject ObjectField(JNIEnv *env, jobject obj, const char *name,
                    const char *signature) {
  jfieldID fId = FieldID(env, obj, name, signature);
  if (!fId) {
    jstring jmsg =
        env->NewStringUTF(std::string("failed to get field ID for field name '")
                              .append(name)
                              .append("' with signature '")
                              .append(signature)
                              .append("'")
                              .c_str());
    jthrowable re =
        (jthrowable)NewJavaObject(env, CLS_RE, METHOD_RE_INIT, jmsg);
    env->Throw(re);
    return nullptr;
  }
  jobject fld = env->GetObjectField(obj, fId);
  checkJvmException(std::string("failed get an object from field '")
                        .append(name)
                        .append("'"));
  return fld;
}

jint IntField(JNIEnv *env, jobject obj, const char *name,
              const char *signature) {
  jfieldID fId = FieldID(env, obj, name, signature);
  if (!fId) {
    jstring jmsg =
        env->NewStringUTF(std::string("failed to get field ID for field name '")
                              .append(name)
                              .append("' with signature '")
                              .append(signature)
                              .append("'")
                              .c_str());
    jthrowable re =
        (jthrowable)NewJavaObject(env, CLS_RE, METHOD_RE_INIT, jmsg);
    env->Throw(re);
    return -1;
  }
  jint fld = env->GetIntField(obj, fId);
  checkJvmException(
      std::string("failed get an Int from field '").append(name).append("'"));
  return fld;
}

jmethodID MethodID(JNIEnv *env, const char *method, const char *signature,
                   const char *className) {
  jmethodID result = methodCache[className][method][signature];

  if (!result) {
    jclass cls = findClass(env, className);
    result = env->GetMethodID(cls, method, signature);

    if (!result) {
      checkJvmException(std::string("failed to get method ")
                        .append(className)
                        .append(".")
                        .append(method));
    }
    
    methodCache[className][method][signature] = result;
  }

  return result;
}

jint IntMethod(JNIEnv *env, const char *method, const char *signature,
               const char *className, const jobject object) {
  jmethodID mId = MethodID(env, method, signature, className);
  checkJvmException(std::string("failed to get method ")
                        .append(className)
                        .append(".")
                        .append(method));

  jint res = env->CallIntMethod(object, mId);
  checkJvmException(std::string("failed to call method ")
                        .append(className)
                        .append(".")
                        .append(method)
                        .append(" using signature ")
                        .append(signature));

  return res;
}

jobject ObjectMethod(JNIEnv *env, const char *method, const char *signature,
                     const char *className, const jobject object, ...) {
  jmethodID mId = MethodID(env, method, signature, className);
  checkJvmException(std::string("failed to get method ")
                        .append(className)
                        .append(".")
                        .append(method));

  va_list varargs;
  va_start(varargs, object);
  jobject res = env->CallObjectMethodV(object, mId, varargs);
  va_end(varargs);
  checkJvmException(std::string("failed to get varargs for ")
                        .append(className)
                        .append(".")
                        .append(method));

  return res;
}

void ThrowByName(JNIEnv *env, const char *className, const char *msg) {
  jclass cls = env->FindClass(className);
  if (cls) {
    env->ThrowNew(cls, msg);
  }
}
