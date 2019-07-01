#include <cassert>

#include "jni_utils.h"
#include "org_bblfsh_client_v2_Context.h"
#include "org_bblfsh_client_v2_Context__.h"
#include "org_bblfsh_client_v2_Node.h"
#include "org_bblfsh_client_v2_libuast_Libuast.h"

#include "libuast.h"
#include "libuast.hpp"

// TODO(bzz): double-check and document. Suggestion and more context at
// https://github.com/bblfsh/client-scala/pull/84#discussion_r288347756
JavaVM *jvm;

namespace {
constexpr char nativeContext[] = "nativeContext";

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

jobject asJvmBuffer(uast::Buffer buf) {
  JNIEnv *env = getJNIEnv();
  return env->NewDirectByteBuffer(buf.ptr, buf.size);
}

// ==========================================
// External UAST Context (managed by libuast)
// ==========================================

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

// ================================================
// UAST Node interface (called from libuast)
// ================================================
class Interface;

class Node : public uast::Node<Node *> {
 private:
  Interface *iface;
  jobject obj;  // Node owns a (global) reference
  NodeKind kind;

  jobject keys;
  std::string *str;

  // kindOf returns a kind of a JVM object.
  // Borrows the reference.
  static NodeKind kindOf(jobject obj) {
    JNIEnv *env = getJNIEnv();
    // TODO(bzz): expose JNode.kind & replace type comparison \w a string test
    if (!obj) {
      return NODE_NULL;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JSTR))) {
      return NODE_STRING;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JINT))) {
      return NODE_INT;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JFLT))) {
      return NODE_FLOAT;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JBOL))) {
      return NODE_BOOL;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JUINT))) {
      return NODE_BOOL;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JARR))) {
      return NODE_ARRAY;
    }
    return NODE_OBJECT;
  }

  Node *lookupOrCreate(jobject obj);

 public:
  friend class Interface;
  friend class Context;

  // Node creates a new node associated with a given JVM object and sets the
  // kind. Creates a new global reference.
  Node(Interface *i, NodeKind k, jobject v) : keys(nullptr), str(nullptr) {
    iface = i;
    obj = getJNIEnv()->NewGlobalRef(v);
    kind = k;
  }

  // Node creates a new node associated with a given JVM object and
  // automatically determines the kind. Creates a new global reference.
  Node(Interface *i, jobject v) : keys(nullptr), str(nullptr) {
    iface = i;
    obj = getJNIEnv()->NewGlobalRef(v);
    kind = kindOf(v);
  }

  ~Node() {
    JNIEnv *env = getJNIEnv();
    if (keys) {
      env->DeleteGlobalRef(keys);
      keys = nullptr;
    }
    if (obj) {
      env->DeleteGlobalRef(obj);
    }
    if (str) {
      delete str;
    }
  }

  jobject toJ();

  NodeKind Kind() { return kind; }

  // TODO(#90) all 'As*' are unused stubs, make them buletprof and test
  std::string *AsString() {
    if (!str) {
      JNIEnv *env = getJNIEnv();
      const char *utf = env->GetStringUTFChars((jstring)obj, 0);
      str = new std::string(utf);
      env->ReleaseStringUTFChars((jstring)obj, utf);
    }

    std::string *s = new std::string(*str);
    return s;
  }
  int64_t AsInt() {
    JNIEnv *env = getJNIEnv();
    jclass cls = env->FindClass("java/lang/Integer");
    jmethodID valueId = env->GetMethodID(cls, "longValue", "()J");
    long long value = (long long)env->CallLongMethod(obj, valueId);
    return (int64_t)(value);
  }
  uint64_t AsUint() {
    JNIEnv *env = getJNIEnv();
    jclass cls = env->FindClass("java/lang/Integer");
    jmethodID valueId = env->GetMethodID(cls, "intValue", "()I");
    jlong value = env->CallIntMethod(obj, valueId);

    jmethodID mId = env->GetMethodID(cls, "toUnsignedLong", "(I)J");
    jlong v = env->CallLongMethod(obj, mId, value);

    return (uint64_t)(v);
  }
  double AsFloat() {
    JNIEnv *env = getJNIEnv();
    jclass cls = env->FindClass("java/lang/Double");
    jmethodID valueId = env->GetMethodID(cls, "floatValue", "()F");
    float value = (float)env->CallFloatMethod(obj, valueId);
    return value;
  }
  bool AsBool() {
    JNIEnv *env = getJNIEnv();
    // TODO(bzz) check failures, cache classes, read 'value' filed
    jclass cls = env->FindClass("java/lang/Boolean");
    jmethodID valueId = env->GetMethodID(cls, "booleanValue", "()Z");
    bool value = (bool)env->CallBooleanMethod(obj, valueId);
    return value;
  }
  size_t Size() {
    jint size = IntMethod(getJNIEnv(), "size", "()I", CLS_JNODE, &obj);
    assert(int32_t(size) >= 0);

    return size;
  }
  std::string *KeyAt(size_t i) {
    if (!obj || i >= Size()) return nullptr;

    JNIEnv *env = getJNIEnv();
    jstring key = (jstring)ObjectMethod(env, "keyAt", METHOD_JNODE_KEY_AT,
                                        CLS_JNODE, &obj);

    const char *k = env->GetStringUTFChars(key, 0);
    std::string *s = new std::string(k);
    env->ReleaseStringUTFChars(key, k);

    return s;
  }
  Node *ValueAt(size_t i) {
    if (!obj || i >= Size()) return nullptr;

    JNIEnv *env = getJNIEnv();
    jobject val =
        ObjectMethod(env, "valueAt", METHOD_JNODE_VALUE_AT, CLS_JNODE, &obj);
    return lookupOrCreate(env->NewGlobalRef(val));  // new ref
  }

  void SetValue(size_t i, Node *val) {
    jobject v = nullptr;
    if (val && val->obj) {
      v = val->obj;
    } else {  // FIXME(bzz)
      //   v = "new JNull instance";  // Py_None;
    }

    jobject res =
        ObjectMethod(getJNIEnv(), "add", METHOD_JARR_ADD, CLS_JARR, &obj, v);
    if (res == nullptr) {  // FIXME(bzz) replace by ThrowNew() to JVM
      throw std::runtime_error(std::string("failed to call ")
                                   .append(CLS_JARR)
                                   .append(".add() from Node::SetValue()"));
    }
  }
  void SetKeyValue(std::string key, Node *val) {
    JNIEnv *env = getJNIEnv();
    jobject v = nullptr;
    if (val && val->obj) {
      v = val->obj;
    } else {
      v = NewJavaObject(env, CLS_JNULL, "()V");
    }

    jstring k = env->NewStringUTF(key.data());

    jobject res =
        ObjectMethod(env, "add", METHOD_JOBJ_ADD, CLS_JOBJ, &obj, k, v);
    if (res == nullptr) {  // FIXME(bzz) replace by ThrowNew() to JVM
      throw std::runtime_error(
          std::string("failed to call JObject.add() from Node::SetKeyValue(")
              .append(key)
              .append(")"));
    }
  }
};

class Context;

class Interface : public uast::NodeCreator<Node *> {
 private:
  std::map<jobject, Node *> obj2node;

  // lookupOrCreate either creates a new object or returns existing one.
  // In the second case it creates a new reference.
  Node *lookupOrCreate(jobject obj) {
    if (!obj) return nullptr;

    Node *node = obj2node[obj];
    if (node) return node;

    node = new Node(this, obj);
    obj2node[obj] = node;
    return node;
  }

  // create makes a new object with a specified kind.
  // Steals the reference.
  Node *create(NodeKind kind, jobject obj) {
    Node *node = new Node(this, kind, obj);
    obj2node[obj] = node;
    return node;
  }

 public:
  friend class Node;
  friend class Context;

  Interface() {}
  ~Interface() {
    // Only needs to deallocate Nodes, since they own
    // the same object as used in the map key.
    for (auto it : obj2node) {
      delete (it.second);
    }
  }

  // toJ returns a JVM object associated with a node.
  // Returns a new reference.
  jobject toJ(Node *node) {
    if (node == nullptr) return nullptr;
    jobject obj = getJNIEnv()->NewGlobalRef(node->obj);
    return obj;
  }

  // abstract methods from NodeCreator
  Node *NewObject(size_t size) {
    jobject m = NewJavaObject(getJNIEnv(), CLS_JOBJ, "()V");
    checkJvmException("failed to create new " + std::string(CLS_JOBJ));
    return create(NODE_OBJECT, m);
  }
  Node *NewArray(size_t size) {
    jobject arr = NewJavaObject(getJNIEnv(), CLS_JARR, "(I)V", size);
    checkJvmException("failed to create new " + std::string(CLS_JARR));
    return create(NODE_ARRAY, arr);
  }
  Node *NewString(std::string v) {
    JNIEnv *env = getJNIEnv();
    jobject str = env->NewStringUTF(v.data());
    jobject arr = NewJavaObject(env, CLS_JSTR, "(Ljava/lang/String;)V", str);
    checkJvmException("failed to create new " + std::string(CLS_JSTR));
    return create(NODE_STRING, arr);
  }
  Node *NewInt(int64_t v) {
    jobject i = NewJavaObject(getJNIEnv(), CLS_JINT, "(J)V", v);
    checkJvmException("failed to create new " + std::string(CLS_JINT));
    return create(NODE_INT, i);
  }
  Node *NewUint(uint64_t v) {
    jobject i = NewJavaObject(getJNIEnv(), CLS_JUINT, "(J)V", v);
    checkJvmException("failed to create new " + std::string(CLS_JUINT));
    return create(NODE_UINT, i);
  }
  Node *NewFloat(double v) {
    jobject i = NewJavaObject(getJNIEnv(), CLS_JFLT, "(D)V", v);
    checkJvmException("failed to create new " + std::string(CLS_JFLT));
    return create(NODE_FLOAT, i);
  }
  Node *NewBool(bool v) {
    jobject i = NewJavaObject(getJNIEnv(), CLS_JBOL, "(Z)V", v);
    checkJvmException("failed to create new " + std::string(CLS_JBOL));
    return create(NODE_BOOL, i);
  }
};

// toJ returns a JVM object associated with a node.
// Returns a new reference.
jobject Node::toJ() { return iface->toJ(this); }

// lookupOrCreate either creates a new object or returns existing one.
// In the second case it creates a new reference.
Node *Node::lookupOrCreate(jobject obj) { return iface->lookupOrCreate(obj); }

class Context {
 private:
  Interface *iface;
  uast::PtrInterface<Node *> *impl;
  uast::Context<Node *> *ctx;

  // toJ returns a JVM object associated with a node.
  // Returns a new reference.
  jobject toJ(Node *node) {
    if (node == nullptr) return nullptr;
    return iface->toJ(node);
  }

 public:
  Context() {
    // create a class that makes and tracks UAST nodes
    iface = new Interface();
    // create an implementation that will handle libuast calls
    impl = new uast::PtrInterface<Node *>(iface);
    // create a new UAST context based on this implementation
    ctx = impl->NewContext();
  }
  ~Context() {
    delete (ctx);
    delete (impl);
    delete (iface);
  }

  // RootNode returns a root UAST node, if set.
  // Returns a new reference.
  jobject RootNode() {
    Node *root = ctx->RootNode();
    return toJ(root);  // new ref
  }

  jobject LoadFrom(jobject src) {  // JNode
    JNIEnv *env = getJNIEnv();

    ContextExt *nodeExtCtx = getHandle<ContextExt>(env, src, "ctx");
    if (!nodeExtCtx) {
      throw std::runtime_error("Cannot get Node.ctx");
    }
    auto sctx = nodeExtCtx->ctx;
    NodeHandle snode =
        reinterpret_cast<NodeHandle>(getHandle<NodeHandle>(env, src, "handle"));
    if (!snode) {
      throw std::runtime_error("Cannot get Node.handle");
    }

    Node *node = uast::Load(sctx, snode, ctx);
    if (!node) {
      throw std::runtime_error("Failed to uast::Load()");
    }
    return toJ(node);  // new ref
  }
};

}  // namespace

// ==========================================
//          v2.libuast.Libuast()
// ==========================================

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
    delete (p);
    env->ExceptionDescribe();
    throw std::runtime_error("failed to instantiate Context class");
  }

  return jCtxExt;
}

// TODO(#86): implement
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_libuast_Libuast_filter(
    JNIEnv *, jobject, jobject, jstring) {
  return nullptr;
}

// ==========================================
//              v2.Context()
// ==========================================

JNIEXPORT jlong JNICALL
Java_org_bblfsh_client_v2_Context_00024_create(JNIEnv *env, jobject self) {
  auto c = new Context();
  uast::Context<NodeHandle> *ctx;  // TODO(#90): init from c on encode() impl
  auto p = new ContextExt(ctx);
  return (long)p;
}

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

// ==========================================
//                v2.Node()
// ==========================================

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_Node_load(JNIEnv *env,
                                                              jobject self) {
  auto ctx = new Context();
  jobject node = ctx->LoadFrom(self);
  delete (ctx);
  return node;
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_8) != JNI_OK) {
    return -1;
  }
  jvm = vm;

  return JNI_VERSION_1_8;
}
