#include <cassert>

#include "jni_utils.h"
#include "org_bblfsh_client_v2_Context.h"
#include "org_bblfsh_client_v2_ContextExt.h"
#include "org_bblfsh_client_v2_Context__.h"
#include "org_bblfsh_client_v2_NodeExt.h"
#include "org_bblfsh_client_v2_libuast_Libuast.h"
#include "org_bblfsh_client_v2_libuast_Libuast_UastIter.h"
#include "org_bblfsh_client_v2_libuast_Libuast_UastIterExt.h"

#include "libuast.h"
#include "libuast.hpp"

// TODO(bzz): double-check and document. Suggestion and more context at
// https://github.com/bblfsh/scala-client/pull/84#discussion_r288347756
JavaVM *jvm;

namespace {
constexpr char nativeContext[] = "nativeContext";

// Reads the opaque native pointer out of the field of the given object.
//
// The field is specified by its name and signature.
// Opaque pointer is casted to the given native type T.
template <typename T>
T *getHandle(JNIEnv *env, jobject obj, const char *name, const char *sig) {
  jfieldID fId = FieldID(env, obj, name, sig);
  if (!fId) return nullptr;

  jlong handle = env->GetLongField(obj, fId);
  checkJvmException("failed to get long field " + std::string(name));
  return reinterpret_cast<T *>(handle);
}

// Reads the opaque native pointer out of the given object's field.
//
// The field is specified by its name.
// Opaque pointer is casted to the given native type T.
template <typename T>
T *getHandle(JNIEnv *env, jobject obj, const char *name) {
  return getHandle<T>(env, obj, name, "J");
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t, const char *name,
               const char *sig) {
  jfieldID fId = FieldID(env, obj, name, sig);
  if (!fId) return;

  jlong handle = reinterpret_cast<jlong>(t);
  env->SetLongField(obj, fId, handle);
}

template <typename T>
void setHandle(JNIEnv *env, jobject obj, T *t, const char *name) {
  setHandle<T>(env, obj, t, name, "J");
}

jobject asJvmBuffer(uast::Buffer buf) {
  JNIEnv *env = getJNIEnv();
  return env->NewDirectByteBuffer(buf.ptr, buf.size);
}

// Checks if a given object is of Node/JNode class
bool isContext(jobject obj, JNIEnv *env) {
  if (!obj) return false;

  jclass ctxCls = env->FindClass(CLS_CTX);
  checkJvmException("failed to find class " + std::string(CLS_CTX));

  return env->IsInstanceOf(obj, ctxCls);
}

bool assertNotContext(jobject obj) {
  JNIEnv *env = getJNIEnv();
  if (isContext(obj, env)) {
    auto reCls = env->FindClass(CLS_RE);
    checkJvmException("failed to find class " + std::string(CLS_RE));

    env->ThrowNew(reCls, "cannot use UAST Context as a Node");
    return false;
  }
  return true;
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

  // toHandle casts an object to NodeExt and reads its handle field.
  // Borrows the reference.
  NodeHandle toHandle(jobject obj) {
    if (!obj) return 0;

    JNIEnv *env = getJNIEnv();  // TODO: refactor to JNI util LongField()
    jclass cls = env->FindClass(CLS_NODE);
    checkJvmException("failed to find class " + std::string(CLS_NODE));

    if (!env->IsInstanceOf(obj, cls)) {
      auto err = std::string("ContextExt.toHandle() argument is not")
                     .append(CLS_NODE)
                     .append(" type");
      ctx->SetError(err);
      return 0;
    }

    jfieldID fId = FieldID(env, obj, "handle", "J");
    if (!fId) {
      return 0;
    }

    auto handle = (NodeHandle)env->GetLongField(obj, fId);
    checkJvmException("failed to get field NodeExt.handle");

    return handle;
  }

 public:
  friend class Context;

  ContextExt(uast::Context<NodeHandle> *c) : ctx(c) {}

  ~ContextExt() { delete (ctx); }

  // lookup searches for a specific node handle.
  jobject lookup(NodeHandle node) { return toJ(node); }

  jobject RootNode() {
    NodeHandle root = ctx->RootNode();
    return lookup(root);
  }

  // Iterate returns iterator over an external UAST tree.
  // Borrows the reference.
  uast::Iterator<NodeHandle> *Iterate(jobject node, TreeOrder order) {
    if (!assertNotContext(node)) return nullptr;

    NodeHandle h = toHandle(node);
    auto iter = ctx->Iterate(h, order);
    return iter;
  }

  // Filter queries an external UAST.
  // Borrows the reference.
  uast::Iterator<NodeHandle> *Filter(jobject node, std::string query) {
    if (!assertNotContext(node)) return nullptr;

    NodeHandle unode = toHandle(node);
    if (unode == 0) unode = ctx->RootNode();

    auto it = ctx->Filter(unode, query);
    return it;
  }

  // Encode serializes the external UAST.
  // Borrows the reference.
  jobject Encode(jobject node, UastFormat format) {
    if (!assertNotContext(node)) return nullptr;

    uast::Buffer data = ctx->Encode(toHandle(node), format);
    return asJvmBuffer(data);
  }
};

// creates new UastIterExt from the given context
jobject filterUastIterExt(ContextExt *ctx, jstring jquery, JNIEnv *env) {
  const char *q = env->GetStringUTFChars(jquery, 0);
  std::string query = std::string(q);
  env->ReleaseStringUTFChars(jquery, q);

  auto node = ctx->RootNode();
  uast::Iterator<NodeHandle> *it = nullptr;
  try {
    it = ctx->Filter(node, query);
  } catch (const std::exception &e) {
    ThrowByName(env, CLS_RE, e.what());
    return nullptr;
  }

  // new UastIterExt()
  jobject iter = NewJavaObject(env, CLS_ITER, METHOD_ITER_INIT, 0, 0, it, ctx);
  if (env->ExceptionCheck() || !iter) {
    delete (it);
    checkJvmException("failed create new UastIterExt class");
  }
  return iter;
}

// ================================================
// UAST Node interface (called from libuast)
// ================================================
class Interface;

class Node : public uast::Node<Node *> {
 private:
  Interface *iface;
  jobject obj;  // Node owns a (global) reference
  NodeKind kind;

  std::string *str;

  // kindOf returns a kind of a JVM object.
  // Borrows the reference.
  static NodeKind kindOf(jobject obj) {
    JNIEnv *env = getJNIEnv();
    // TODO(bzz): expose JNode.kind & replace type comparison \w a string test
    if (!obj || env->IsInstanceOf(obj, env->FindClass(CLS_JNULL))) {
      return NODE_NULL;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JSTR))) {
      return NODE_STRING;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JINT))) {
      return NODE_INT;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JFLT))) {
      return NODE_FLOAT;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JBOOL))) {
      return NODE_BOOL;
    } else if (env->IsInstanceOf(obj, env->FindClass(CLS_JUINT))) {
      return NODE_UINT;
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
  Node(Interface *i, NodeKind k, jobject v) : str(nullptr) {
    iface = i;
    obj = getJNIEnv()->NewGlobalRef(v);
    kind = k;
  }

  // Node creates a new node associated with a given JVM object and
  // automatically determines the kind. Creates a new global reference.
  Node(Interface *i, jobject v) : str(nullptr) {
    iface = i;
    obj = getJNIEnv()->NewGlobalRef(v);
    kind = kindOf(v);
  }

  ~Node() {
    JNIEnv *env = getJNIEnv();
    if (obj) {
      env->DeleteGlobalRef(obj);
    }
    if (str) {
      delete str;
    }
  }

  jobject toJ();

  NodeKind Kind() { return kind; }

  std::string *AsString() {  // new ref
    if (!str) {
      const char methodName[] = "str";
      JNIEnv *env = getJNIEnv();
      jstring jstr = (jstring)ObjectMethod(
          env, methodName, "()Ljava/lang/String;", CLS_JSTR, &obj);

      const char *utf = env->GetStringUTFChars(jstr, 0);
      str = new std::string(utf);
      env->ReleaseStringUTFChars(jstr, utf);
    }

    std::string *s = new std::string(*str);
    return s;
  }
  int64_t AsInt() {
    const char methodName[] = "num";
    JNIEnv *env = getJNIEnv();
    jmethodID mID = MethodID(env, methodName, "()J", CLS_JINT);

    long long value = (long long)env->CallLongMethod(obj, mID);
    checkJvmException(std::string("failed to call ")
                          .append(CLS_JINT)
                          .append(".")
                          .append(methodName)
                          .append(" at Node::AsInt()"));
    return (int64_t)(value);
  }
  uint64_t AsUint() {
    const char methodName[] = "get";
    JNIEnv *env = getJNIEnv();
    jmethodID mID = MethodID(env, methodName, "()J", CLS_JUINT);

    jlong value = env->CallLongMethod(obj, mID);
    checkJvmException(std::string("failed to call ")
                          .append(CLS_JUINT)
                          .append(".")
                          .append(methodName)
                          .append(" at Node::AsUint()"));
    return (uint64_t)(value);
  }
  double AsFloat() {
    const char methodName[] = "num";
    JNIEnv *env = getJNIEnv();
    jmethodID mID = MethodID(env, methodName, "()D", CLS_JFLT);

    double value = (double)env->CallDoubleMethod(obj, mID);
    checkJvmException(std::string("failed to call ")
                          .append(CLS_JFLT)
                          .append(".")
                          .append(methodName)
                          .append(" at Node::AsFloat()"));
    return value;
  }
  bool AsBool() {
    const char methodName[] = "value";
    JNIEnv *env = getJNIEnv();
    jmethodID mID = MethodID(env, methodName, "()Z", CLS_JBOOL);

    bool value = (bool)env->CallBooleanMethod(obj, mID);
    checkJvmException(std::string("failed to call ")
                          .append(CLS_JBOOL)
                          .append(".")
                          .append(methodName)
                          .append(" at Node::AsBool()"));
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
                                        CLS_JNODE, &obj, i);

    const char *k = env->GetStringUTFChars(key, 0);
    std::string *s = new std::string(k);
    env->ReleaseStringUTFChars(key, k);

    return s;
  }
  Node *ValueAt(size_t i) {
    if (!obj || i >= Size()) return nullptr;

    JNIEnv *env = getJNIEnv();
    jobject val =
        ObjectMethod(env, "valueAt", METHOD_JNODE_VALUE_AT, CLS_JNODE, &obj, i);
    return lookupOrCreate(env->NewGlobalRef(val));  // new ref
  }

  void SetValue(size_t i, Node *val) {
    JNIEnv *env = getJNIEnv();
    jobject v = nullptr;
    if (val && val->obj) {
      v = val->obj;
    } else {
      v = NewJavaObject(env, CLS_JNULL, "()V");
    }

    ObjectMethod(getJNIEnv(), "add", METHOD_JARR_ADD, CLS_JARR, &obj, v);
    checkJvmException(std::string("failed to call ")
                          .append(CLS_JARR)
                          .append(".add() from Node::SetValue()"));
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

    ObjectMethod(env, "add", METHOD_JOBJ_ADD, CLS_JOBJ, &obj, k, v);
    checkJvmException(
        std::string("failed to call JObject.add() from Node::SetKeyValue(")
            .append(key)
            .append(")"));
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
    if (!node) return nullptr;
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
    jobject i = NewJavaObject(getJNIEnv(), CLS_JBOOL, "(Z)V", v);
    checkJvmException("failed to create new " + std::string(CLS_JBOOL));
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
    if (!node) return nullptr;
    return iface->toJ(node);
  }
  // toNode returns a node associated with a JVM object.
  // Returns a new reference.
  Node *toNode(jobject jnode) { return iface->lookupOrCreate(jnode); }

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

  // Iterate returns iterator over an external UAST tree.
  // Creates a new reference.
  uast::Iterator<Node *> *Iterate(jobject jnode, TreeOrder order) {
    if (!assertNotContext(jnode)) return nullptr;

    Node *n = toNode(jnode);
    auto iter = ctx->Iterate(n, order);
    return iter;
  }

  // Filter queries UAST.
  // Creates a new reference.
  uast::Iterator<Node *> *Filter(jobject node, std::string query) {
    if (!assertNotContext(node)) return nullptr;

    Node *unode = toNode(node);
    if (unode == nullptr) unode = ctx->RootNode();

    auto it = ctx->Filter(unode, query);
    return it;
  }

  // Encode serializes UAST.
  // Creates a new reference.
  jobject Encode(jobject jnode, UastFormat format) {
    if (!assertNotContext(jnode)) return nullptr;

    Node *n = toNode(jnode);
    uast::Buffer data = ctx->Encode(n, format);
    return asJvmBuffer(data);
  }

  jobject LoadFrom(jobject src) {  // NodeExt
    JNIEnv *env = getJNIEnv();

    ContextExt *nodeExtCtx = getHandle<ContextExt>(env, src, "ctx");
    checkJvmException("failed to get NodeExt.ctx");

    auto sctx = nodeExtCtx->ctx;
    NodeHandle snode =
        reinterpret_cast<NodeHandle>(getHandle<NodeHandle>(env, src, "handle"));
    checkJvmException("failed to get NodeExt.handle");

    Node *node = uast::Load(sctx, snode, ctx);
    return toJ(node);
  }
};

}  // namespace

// ==========================================
//          v2.libuast.Libuast
// ==========================================

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_libuast_Libuast_decode(
    JNIEnv *env, jobject self, jobject directBuf) {
  UastFormat format = UAST_BINARY;  // TODO: make it arg

  // works only with ByteBuffer.allocateDirect()
  void *buf = env->GetDirectBufferAddress(directBuf);
  checkJvmException("failed to use buffer for direct access");

  jlong len = env->GetDirectBufferCapacity(directBuf);
  checkJvmException("failed to get buffer capacity");

  // another option (instead of XXX) is to use
  // GetPrimitiveArrayCritical
  uast::Buffer ubuf(buf, (size_t)(len));
  uast::Context<NodeHandle> *ctx = uast::Decode(ubuf, format);
  // ReleasePrimitiveArrayCritical

  auto p = new ContextExt(ctx);

  jobject jCtxExt = NewJavaObject(env, CLS_CTX, "(J)V", p);
  if (env->ExceptionCheck() || !jCtxExt) {
    jCtxExt = nullptr;
    delete (ctx);
    delete (p);
    checkJvmException("failed to instantiate ContextExt class");
  }

  return jCtxExt;
}

// UastIter
JNIEXPORT void JNICALL
Java_org_bblfsh_client_v2_libuast_Libuast_00024UastIter_nativeInit(
    JNIEnv *env, jobject self) {
  jobject jnode = ObjectField(env, self, "node", FIELD_ITER_NODE);
  if (!jnode) {
    return;
  }

  Context *ctx = new Context();
  jint order = IntField(env, self, "treeOrder", "I");
  if (order < 0) {
    return;
  }

  // global ref will be deleted by Interface destructor on ctx deletion
  auto it = ctx->Iterate(env->NewGlobalRef(jnode), (TreeOrder)order);

  // this.iter = it;
  setHandle<uast::Iterator<Node *>>(env, self, it, "iter");
  // this.ctx = ctx;
  setHandle<Context>(env, self, ctx, "ctx");
  return;
}

JNIEXPORT void JNICALL
Java_org_bblfsh_client_v2_libuast_Libuast_00024UastIter_nativeDispose(
    JNIEnv *env, jobject self) {
  // this.ctx - delete as iterator owns it for both .iterate()/.filter() cases
  auto ctx = getHandle<Context>(env, self, "ctx");
  delete (ctx);
  setHandle<Context>(env, self, 0, "ctx");

  // this.iter
  auto iter = getHandle<uast::Iterator<Node *>>(env, self, "iter");
  setHandle<uast::Iterator<Node *>>(env, self, 0, "iter");
  delete (iter);
  return;
}

JNIEXPORT jobject JNICALL
Java_org_bblfsh_client_v2_libuast_Libuast_00024UastIter_nativeNext(
    JNIEnv *env, jobject self, jlong iterPtr) {
  // this.iter
  auto iter = reinterpret_cast<uast::Iterator<Node *> *>(iterPtr);

  try {
    if (!iter->next()) {
      return nullptr;
    }
  } catch (const std::exception &e) {
    ThrowByName(env, CLS_RE, e.what());
    return nullptr;
  }

  Node *node = iter->node();
  if (!node) return nullptr;

  return node->toJ();  // new global ref
}

// UastIterExt
JNIEXPORT void JNICALL
Java_org_bblfsh_client_v2_libuast_Libuast_00024UastIterExt_nativeInit(
    JNIEnv *env, jobject self) {  // sets iter and ctx, given node: NodeExt

  jobject nodeExt = ObjectField(env, self, "node", FIELD_ITER_NODE);
  if (!nodeExt) {
    return;
  }

  // borrow ContextExt from NodeExt
  ContextExt *ctx = getHandle<ContextExt>(env, nodeExt, "ctx");

  jint order = IntField(env, self, "treeOrder", "I");
  if (order < 0) {
    return;
  }

  auto it = ctx->Iterate(nodeExt, (TreeOrder)order);

  // this.iter = it;
  setHandle<uast::Iterator<NodeHandle>>(env, self, it, "iter");
  // this.ctx = ctx;
  setHandle<ContextExt>(env, self, ctx, "ctx");
  return;
}

JNIEXPORT void JNICALL
Java_org_bblfsh_client_v2_libuast_Libuast_00024UastIterExt_nativeDispose(
    JNIEnv *env, jobject self) {
  // this.ctx - don't delete ContextExt, it's borrowed from NodeExt
  setHandle<ContextExt>(env, self, 0, "ctx");

  // this.iter
  auto iter = getHandle<uast::Iterator<NodeHandle>>(env, self, "iter");
  setHandle<uast::Iterator<NodeHandle>>(env, self, 0, "iter");
  delete (iter);
  return;
}

JNIEXPORT jobject JNICALL
Java_org_bblfsh_client_v2_libuast_Libuast_00024UastIterExt_nativeNext(
    JNIEnv *env, jobject self, jlong iterPtr) {
  // this.iter
  auto iter = reinterpret_cast<uast::Iterator<NodeHandle> *>(iterPtr);

  try {
    if (!iter->next()) {
      return nullptr;
    }
  } catch (const std::exception &e) {
    ThrowByName(env, CLS_RE, e.what());
    return nullptr;
  }

  NodeHandle node = iter->node();
  if (node == 0) return nullptr;

  ContextExt *ctx = getHandle<ContextExt>(env, self, "ctx");
  return ctx->lookup(node);
}

// ==========================================
//              v2.Context()
// ==========================================

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_Context_filter(
    JNIEnv *env, jobject self, jstring jquery, jobject jnode) {
  Context *ctx = getHandle<Context>(env, self, nativeContext);

  const char *q = env->GetStringUTFChars(jquery, 0);
  std::string query = std::string(q);
  env->ReleaseStringUTFChars(jquery, q);

  uast::Iterator<Node *> *it = nullptr;
  try {
    // global ref will be deleted by Interface destructor on ctx deletion
    it = ctx->Filter(env->NewGlobalRef(jnode), query);
  } catch (const std::exception &e) {
    ThrowByName(env, CLS_RE, e.what());
    return nullptr;
  }

  // new UastIter()
  jobject iter =
      NewJavaObject(env, CLS_JITER, METHOD_JITER_INIT, 0, 0, it, ctx);
  if (env->ExceptionCheck() || !iter) {
    delete (it);
    checkJvmException("failed create new UastIter class");
  }
  return iter;
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_Context_encode(
    JNIEnv *env, jobject self, jobject jnode) {
  UastFormat fmt = UAST_BINARY;  // TODO(#107): make it argument

  Context *p = getHandle<Context>(env, self, nativeContext);
  return p->Encode(jnode, fmt);
}

JNIEXPORT jlong JNICALL
Java_org_bblfsh_client_v2_Context_00024_create(JNIEnv *env, jobject self) {
  auto c = new Context();
  return (long)c;
}

JNIEXPORT void JNICALL Java_org_bblfsh_client_v2_Context_dispose(JNIEnv *env,
                                                                 jobject self) {
  Context *p = getHandle<Context>(env, self, nativeContext);
  setHandle<Context>(env, self, 0, nativeContext);
  delete p;
};

// ==========================================
//              v2.ContextExt()
// ==========================================

JNIEXPORT jobject JNICALL
Java_org_bblfsh_client_v2_ContextExt_root(JNIEnv *env, jobject self) {
  ContextExt *p = getHandle<ContextExt>(env, self, nativeContext);
  return p->RootNode();
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_ContextExt_filter(
    JNIEnv *env, jobject self, jstring jquery) {
  ContextExt *ctx = getHandle<ContextExt>(env, self, nativeContext);
  return filterUastIterExt(ctx, jquery, env);
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_ContextExt_encode(
    JNIEnv *env, jobject self, jobject node) {
  UastFormat fmt = UAST_BINARY;  // TODO(#107): make it argument

  ContextExt *p = getHandle<ContextExt>(env, self, nativeContext);
  return p->Encode(node, fmt);
}

JNIEXPORT void JNICALL
Java_org_bblfsh_client_v2_ContextExt_dispose(JNIEnv *env, jobject self) {
  ContextExt *p = getHandle<ContextExt>(env, self, nativeContext);
  setHandle<ContextExt>(env, self, 0, nativeContext);
  delete p;
}

// ==========================================
//                v2.Node()
// ==========================================

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_NodeExt_load(JNIEnv *env,
                                                                 jobject self) {
  auto ctx = new Context();
  jobject node = ctx->LoadFrom(self);
  delete (ctx);
  return node;
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_NodeExt_filter(
    JNIEnv *env, jobject self, jstring jquery) {
  auto *ctx = getHandle<ContextExt>(env, self, "ctx");
  return filterUastIterExt(ctx, jquery, env);
}

JNIEXPORT jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  JNIEnv *env;
  if (vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_8) != JNI_OK) {
    return JNI_ERR;
  }
  jvm = vm;

  return JNI_VERSION_1_8;
}
