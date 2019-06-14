#include "org_bblfsh_client_v2_libuast_Libuast.h"
#include "jni_utils.h"
#include "org_bblfsh_client_v2_Context.h"

#include "libuast.h"
#include "libuast.hpp"

// TODO(bzz): double-check and document. Suggestion and more context at
// https://github.com/bblfsh/client-scala/pull/84#discussion_r288347756
JavaVM *jvm;

namespace {
jobject asJvmBuffer(uast::Buffer buf) {
  JNIEnv *env = getJNIEnv();
  return env->NewDirectByteBuffer(buf.ptr, buf.size);
}

const char *nativeContext = "nativeContext";

jfieldID getHandleField(JNIEnv *env, jobject obj, const char *name) {
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

template <typename T>
T *getHandle(JNIEnv *env, jobject obj, const char *name) {
  jlong handle = env->GetLongField(obj, getHandleField(env, obj, name));
  if (env->ExceptionOccurred() || !handle) {
    return nullptr;
  }
  return reinterpret_cast<T *>(handle);
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t, const char *name) {
  jlong handle = reinterpret_cast<jlong>(t);
  env->SetLongField(obj, getHandleField(env, obj, name), handle);
}

class ContextExt {
 private:
  uast::Context<NodeHandle> *ctx;

  jobject toJ(NodeHandle node) {
    if (node == 0) return nullptr;

    JNIEnv *env = getJNIEnv();
    jobject jObj = NewJavaObject(env, CLS_NODE, "(JJ)V", this, node);
    return jObj;
  }

  // toHandle casts an object to NodeExt and returns its handle.
  // Borrows the reference.
  NodeHandle toHandle(jobject obj) {
    if (!obj) return 0;

    JNIEnv *env = getJNIEnv();
    jclass cls = env->FindClass(CLS_NODE);
    if (env->ExceptionOccurred() || !cls) {
      return 0;
    }

    if (!env->IsInstanceOf(obj, cls)) {
      const char *err = "ContextExt.toHandle() called not on Node type";
      ctx->SetError(err);
      return 0;
    }

    auto handle =
        (NodeHandle)env->GetLongField(obj, getField(env, obj, "handle"));
    if (env->ExceptionOccurred() || !handle) {
      return 0;
    }

    return handle;
  }

 public:
  friend class Context;

  ContextExt(uast::Context<NodeHandle> *c) : ctx(c) {}

  ~ContextExt() { delete (ctx); }

  jobject RootNode() {
    NodeHandle root = ctx->RootNode();
    return toJ(root);
  }

  // Encode serializes external UAST.
  // Borrows the reference.
  jobject Encode(jobject node, UastFormat format) {
    NodeHandle h = toHandle(node);
    uast::Buffer data = ctx->Encode(h, format);
    return asJvmBuffer(data);
  }
};
}  // namespace

// v2.libuast.Libuast()
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_libuast_Libuast_decode(
    JNIEnv *env, jobject self, jobject directBuf) {
  UastFormat format = UAST_BINARY;  // TODO: make it arg

  // works only with ByteBuffer.allocateDirect()
  void *buf = env->GetDirectBufferAddress(directBuf);
  if (env->ExceptionCheck() == JNI_TRUE) {
    return nullptr;
  }
  jlong len = env->GetDirectBufferCapacity(directBuf);
  if (env->ExceptionCheck() == JNI_TRUE) {
    return nullptr;
  }

  // another option (instead of XXX) is to use
  // GetPrimitiveArrayCritical
  uast::Buffer ubuf(buf, (size_t)(len));
  uast::Context<NodeHandle> *ctx = uast::Decode(ubuf, format);
  // ReleasePrimitiveArrayCritical

  auto p = new ContextExt(ctx);

  jobject jCtxExt = NewJavaObject(env, CLS_CTX, "(J)V", p);
  if (env->ExceptionCheck() == JNI_TRUE || !jCtxExt) {
    jCtxExt = nullptr;
    delete (ctx);
    env->ExceptionDescribe();
    throw std::runtime_error("failed to instantiate Context class");
  }

  return jCtxExt;
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_libuast_Libuast_filter(
    JNIEnv *, jobject, jobject, jstring) {
  return nullptr;  // TODO(bzz): implement
}

// v2.Context()
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_Context_root(JNIEnv *env,
                                                                 jobject self) {
  ContextExt *p = getHandle<ContextExt>(env, self, nativeContext);
  return p->RootNode();
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_Context_encode(
    JNIEnv *env, jobject self, jobject node) {
  UastFormat fmt = UAST_BINARY;  // TODO(bzz): make it argument & enum

  ContextExt *p = getHandle<ContextExt>(env, self, nativeContext);
  return p->Encode(node, fmt);
}

JNIEXPORT void JNICALL Java_org_bblfsh_client_v2_Context_dispose(JNIEnv *env,
                                                                 jobject self) {
  ContextExt *p = getHandle<ContextExt>(env, self, nativeContext);
  setHandle<ContextExt>(env, self, 0, nativeContext);
  delete p;
}

// v2.Node()
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_Node_load(JNIEnv *,
                                                              jobject) {
  return nullptr;  // TODO(bzz): implement
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_8) != JNI_OK) {
    return -1;
  }
  jvm = vm;

  return JNI_VERSION_1_8;
}
