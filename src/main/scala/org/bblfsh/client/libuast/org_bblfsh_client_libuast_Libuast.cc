#include "org_bblfsh_client_libuast_Libuast.h"
#include "jni_utils.h"
#include "handle.h"
#include "memtracker.h"

#include <cstring>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <vector>

#include "libuast.h"
#include "libuast.hpp"


JavaVM *jvm;
// Keep track of the current iterator for copyRefNode (since we can't pass the iter as
// argument)
MemTracker memTracker;

static void *copyRefNode(void *node) {
  JNIEnv *env = getJNIEnv();
  // jobject itself is a opaque struct with a pointer and the pointed obj is deallocated
  // at the end so we need to increase the global ref and copy the returned jobject
  jobject ref = env->NewGlobalRef(*((jobject*)node));
  if (env->ExceptionCheck() == JNI_TRUE || !ref) {
    return NULL;
  }

  void *nodeptr = (void*)malloc(sizeof(jobject));
  memcpy(nodeptr, &ref, sizeof(jobject));

  memTracker.AddIterRefNode(nodeptr, ref);
  return nodeptr;
}


jobject asJvmBuffer(uast::Buffer buf) {
  JNIEnv *env = getJNIEnv();
  return env->NewDirectByteBuffer(buf.ptr, buf.size);
}


class Interface;

class Node : public uast::Node<Node*> {
private:
    Interface* ctx;
    jobject obj; // Node owns a reference
    NodeKind  kind;

    jobject keys;
    std::string* str;

    // checkJvmException checks for a JVM exceptions, and if any, throws an error.
    static void checkJvmException(std::string msg) {
        JNIEnv *env = getJNIEnv();
        if (env->ExceptionOccurred()) {
            auto err = env->ExceptionOccurred();
            jboolean isCopy = false;
            jmethodID toString = env->GetMethodID(env->FindClass("java/lang/Object"), "toString", "()Ljava/lang/String;");
            jstring s = (jstring)env->CallObjectMethod(err, toString);
            const char* utf = env->GetStringUTFChars(s, &isCopy);
            std::string* str = new std::string(utf);
            env->ReleaseStringUTFChars(s, utf);
            throw std::runtime_error(msg + *str); //or ThrowNew()?
        }
    }

    // kindOf returns a kind of a Python object.
    // Borrows the reference.
    static NodeKind kindOf(jobject obj) {
        JNIEnv *env = getJNIEnv();
        if (!obj) {
          return NODE_NULL;
        } else if (env->IsInstanceOf(obj, env->FindClass("java/lang/String"))) {
          return NODE_STRING;
        } else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Integer"))) {
          return NODE_INT;
        } else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Float"))) {
          return NODE_FLOAT;
        } else if (env->IsInstanceOf(obj, env->FindClass("java/lang/Boolean"))) {
          return NODE_BOOL;
        } else if (env->IsInstanceOf(obj, env->FindClass("java/util/ArrayList"))) {
          return NODE_ARRAY;
        }
        return NODE_OBJECT;
    }
    Node* lookupOrCreate(jobject obj);
public:
    friend class Interface;
    friend class Context;

    // Node creates a new node associated with a given JVM object and sets the kind.
    // Steals the reference.
    Node(Interface* c, NodeKind k, jobject v) : keys(nullptr), str(nullptr) {
        ctx = c;
        obj = v;
        kind = k;
    }
    // Node creates a new node associated with a given JVM object and automatically determines the kind.
    // Creates a new reference.
    Node(Interface* c, jobject v) : keys(nullptr), str(nullptr) {
        ctx = c;
        obj = getJNIEnv()->NewGlobalRef(v);
        kind = kindOf(v);
    }
    ~Node(){
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

    NodeKind Kind() {
        return kind;
    }
    std::string* AsString() {
        if (!str) {
            JNIEnv *env = getJNIEnv();
            jboolean isCopy = false;
            const char* utf = env->GetStringUTFChars((jstring)obj, &isCopy);
            str = new std::string(utf);
            env->ReleaseStringUTFChars((jstring)obj, utf);
        }

        std::string* s = new std::string(*str);
        return s;
    }
    int64_t AsInt() {
        JNIEnv *env = getJNIEnv();
        jclass cls = env->FindClass("java/lang/Integer");
        jmethodID valueId = env->GetMethodID(cls, "longValue", "()J");
        long long value = (long long) env->CallLongMethod(obj, valueId);
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
        float value = (float) env->CallFloatMethod(obj, valueId);
        return value;
    }
    bool AsBool() {
        JNIEnv *env = getJNIEnv();
        //TODO(bzz) check failures, cache classes, read 'value' filed
        jclass cls = env->FindClass("java/lang/Boolean");
        jmethodID valueId = env->GetMethodID(cls, "booleanValue", "()Z");
        bool value = (bool) env->CallBooleanMethod(obj, valueId);
        return value;
    }

    size_t Size() { //not sure why is this needed
        size_t sz = 0;
        JNIEnv *env = getJNIEnv();

        jclass arrCls = env->FindClass("java/util/ArrayList");
        jclass mapCls = env->FindClass("java/util/TreeMap");
        jclass strCls = env->FindClass("java/lang/String");

        if (!obj) {
          sz = 0;
        } else if (env->IsInstanceOf(obj, arrCls)) {
          jmethodID sizeId = env->GetMethodID(arrCls, "size", "()Z");
          sz = env->CallIntMethod(obj, sizeId);
        } else if (env->IsInstanceOf(obj, mapCls)) {
          jmethodID sizeId = env->GetMethodID(mapCls, "size", "()Z");
          sz = env->CallIntMethod(obj, sizeId);
        } else if (env->IsInstanceOf(obj, strCls)) {
          jmethodID sizeId = env->GetMethodID(strCls, "length", "()Z");
          sz = env->CallIntMethod(obj, sizeId);
        }
        //assert(int64_t(sz) >= 0);
        return sz;
    }

    std::string* KeyAt(size_t i) {
        if (!obj) {
            return nullptr;
        }
        JNIEnv *env = getJNIEnv();

        /*TODO(bzz): replace \w TreeMap manipulations
         - iterate ordered KeySet view
         - it.next(); delete LocalRefs passed to JNI

        if (!keys) keys = PyDict_Keys(obj);
        PyObject* key = PyList_GetItem(keys, i); // borrows
        */

        jstring key;
        jboolean isCopy = false;
        const char * k = env->GetStringUTFChars(key, &isCopy);
        std::string* s = new std::string(k);
        env->ReleaseStringUTFChars(key, k);
        return s;
    }
    Node* ValueAt(size_t i) {
        if (!obj) {
            return nullptr;
        }
        JNIEnv *env = getJNIEnv();

        jclass arrCls = env->FindClass("java/util/ArrayList"); //TODO(bzz): cache. this is expensive!
        if (env->IsInstanceOf(obj, arrCls)) {
            jmethodID getId = env->GetMethodID(arrCls, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
            jobject el = env->CallObjectMethod(obj, getId, i);
            jobject v = env->NewGlobalRef(el); // borrows
            return lookupOrCreate(v); // new ref
        }

        /*TODO(bzz): Object case: replace \w TreeMap<String, Object> manipulations

        if (!keys) keys = PyDict_Keys(obj);
        PyObject* key = PyList_GetItem(keys, i); // borrows
        PyObject* val = PyDict_GetItem(obj, key); // borrows
        */
        jobject val;
        return lookupOrCreate(val); // new ref
    }

    void SetValue(size_t i, Node* val) {
        JNIEnv *env = getJNIEnv();
        jobject v = nullptr;
        if (val && val->obj) {
            v = env->NewGlobalRef(val->obj);
        }
        //PyList_SetItem(obj, i, v); // steals
        jclass arrCls = env->FindClass("java/util/ArrayList");
        jmethodID setId = env->GetMethodID(arrCls, "set", "(ILjava/lang/Object;)Ljava/lang/Object");
        jobject el = env->CallObjectMethod(obj, setId, i, v);
    }
    void SetKeyValue(std::string k, Node* val) {
        JNIEnv *env = getJNIEnv();
        jobject v = nullptr;
        if (val && val->obj) {
            v = val->obj;
        }
        //PyDict_SetItemString(obj, k.data(), v); // new ref
        jclass mapCls = env->FindClass("java/util/TreeMap");
        jmethodID putId = env->GetMethodID(mapCls, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object");
        jobject el = env->CallObjectMethod(obj, putId, k.data(), v);
        checkJvmException("Failed to put " + k + " from Node::SetKeyValue");
    }
};



class Context;

class Interface : public uast::NodeCreator<Node*> {
private:
    std::map<jobject, Node*> obj2node;

    // lookupOrCreate either creates a new object or returns existing one.
    // In the second case it creates a new reference.
    Node* lookupOrCreate(jobject obj) {
        if (!obj) return nullptr;

        Node* node = obj2node[obj];
        if (node) return node;

        node = new Node(this, obj);
        obj2node[obj] = node;
        return node;
    }

    // create makes a new object with a specified kind.
    // Steals the reference.
    Node* create(NodeKind kind, jobject obj) {
        Node* node = new Node(this, kind, obj);
        obj2node[obj] = node;
        return node;
    }
public:
    friend class Node;
    friend class Context;

    Interface(){
    }
    ~Interface(){
        // Only needs to deallocate Nodes, since they own
        // the same object as used in the map key.
        for (auto it : obj2node) {
            delete(it.second);
        }
    }

    // toNode creates a new or returns an existing node associated with JVM object.
    // Creates a new reference.
    Node* toNode(jobject obj){
        return lookupOrCreate(obj);
    }

    // toJ returns a JVM object associated with a node.
    // Returns a new reference.
    jobject toJ(Node* node) {
        if (node == nullptr) return nullptr;
        jobject obj = getJNIEnv()->NewGlobalRef(node->obj);
        return obj;
    }

    Node* NewObject(size_t size) {
        JNIEnv *env = getJNIEnv();
        jobject m = NewJavaObject(env, "java/util/TreeMap", "()V");
        return create(NODE_OBJECT, m);
    }
    Node* NewArray(size_t size) {
        JNIEnv *env = getJNIEnv();
        jobject arr = NewJavaObject(env, "java/util/ArrayList", "(I)V", size);
        return create(NODE_ARRAY, arr);
    }
    Node* NewString(std::string v) {
        JNIEnv *env = getJNIEnv();
        jobject obj = env->NewStringUTF(v.data()); //TODO(bzz): NewGlobalRefs here?
        return create(NODE_STRING, obj);
    }
    Node* NewInt(int64_t v) {
        JNIEnv *env = getJNIEnv();
        jobject obj = NewJavaObject(env, "java/lang/Integer", "(I)V", v);
        return create(NODE_INT, obj);
    }
    Node* NewUint(uint64_t v) {
        JNIEnv *env = getJNIEnv();
        jobject obj = NewJavaObject(env, "java/lang/Integer", "(I)V", v);
        return create(NODE_UINT, obj);
    }
    Node* NewFloat(double v) {
        JNIEnv *env = getJNIEnv();
        jobject obj = NewJavaObject(env, "java/lang/Double", "(D)V", v);
        return create(NODE_FLOAT, obj);
    }
    Node* NewBool(bool v) {
        JNIEnv *env = getJNIEnv();
        jobject obj = NewJavaObject(env, "java/lang/Boolean", "(Z)V", v);
        return create(NODE_BOOL, obj);
    }
};


class ContextExt {
private:
    uast::Context<NodeHandle> *ctx;

    jobject toJ(NodeHandle node) {
        if (node == 0) return nullptr;

        JNIEnv *env = getJNIEnv();
        jobject jObj = NewJavaObject(env, CLS_NODE_EXT, "(JJ)V", this, node);

        return jObj;
    }

    // toHandle casts an object to NodeExt and returns its handle.
    // Borrows the reference.
    NodeHandle toHandle(jobject obj) {
        if (!obj) return 0;

        JNIEnv *env = getJNIEnv();
        jclass cls = env->FindClass(CLS_NODE_EXT);
        if (env->ExceptionOccurred() || !cls) {
            return 0;
        }

        if (!env->IsInstanceOf(obj, cls)) {
            const char* err = "ContextExt.toHandle() called not on NodeExt type";
            ctx->SetError(err);
            return 0;
        }

        auto handle = (NodeHandle) env->GetLongField(obj, getHandleField(env, obj, "handle"));
        if (env->ExceptionOccurred() || !handle) {
            return 0;
        }

        return handle;
    }
public:
    friend class Context;

    ContextExt(uast::Context<NodeHandle> *c) : ctx(c) {}

    ~ContextExt(){
        delete(ctx);
    }

    jobject RootNode(){
        NodeHandle root = ctx->RootNode();
        return toJ(root);
    }

    // Encode serializes existing-on-guest-side UAST.
    // Borrows the reference.
    jobject Encode(jobject node, UastFormat format) {
        NodeHandle h = toHandle(node);
        uast::Buffer data = ctx->Encode(h, format);
        return asJvmBuffer(data);
    }
};



class Context {
private:
    Interface *iface;
    uast::PtrInterface<Node*> *impl;
    uast::Context<Node*>   *ctx;

    // toJ returns a JVM object associated with a node.
    // Returns a new reference.
    jobject toJ(Node* node) {
        if (node == nullptr) return nullptr;
        return iface->toJ(node);
    }
public:
    Context(){
        // create a class that makes and tracks UAST nodes
        iface = new Interface();
        // create an implementation that will handle libuast calls
        impl = new uast::PtrInterface<Node*>(iface);
        // create a new UAST context based on this implementation
        ctx = impl->NewContext();
    }
    ~Context(){
        delete(ctx);
        delete(impl);
        delete(iface);
    }

    // RootNode returns a root UAST node, if set.
    // Returns a new reference.
    jobject RootNode(){
        Node* root = ctx->RootNode();
        return toJ(root); // new ref
    }

    jobject LoadFrom(jobject src) { // NodeExt
        JNIEnv *env = getJNIEnv();

        ContextExt *nodeExtCtx = getHandle<ContextExt>(env, src, "ctx");
        if (!nodeExtCtx) {
            throw std::runtime_error("Cannot get NodeExt.ctx");
        }
        auto sctx = nodeExtCtx->ctx;
        NodeHandle *snode = getHandle<NodeHandle>(env, src, "handle");
        if (!snode) {
            throw std::runtime_error("Cannot get NodeExt.handle");
        }

        Node* node = uast::Load(sctx, *snode, ctx);
        if (!node) {
            throw std::runtime_error("Failed to uast::Load()");
        }
        return toJ(node); // new ref
    }
};

// toJ returns a JVM object associated with a node.
// Returns a new reference.
jobject Node::toJ() {
    return ctx->toJ(this);
}

// lookupOrCreate either creates a new object or returns existing one.
// In the second case it creates a new reference.
Node* Node::lookupOrCreate(jobject obj) {
    return ctx->lookupOrCreate(obj);
}

//// Exported Java functions ////

// 00024 = "$" in .class files == Inner class reference
/*
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_newIterator
  (JNIEnv *env, jobject self, jobject obj, int treeOrder) {

    memTracker.ClearCurrentIterator();
    jobject *nodeptr = &obj;
    UastIterator *iter = UastIteratorNewWithTransformer(ctx, nodeptr, (TreeOrder)treeOrder,
                                                        copyRefNode);
    if (env->ExceptionCheck() == JNI_TRUE) {
      return NULL;
    }
    if (!iter) {
      ThrowException(LastError());
      return 0;
    }
    memTracker.SetCurrentIterator(iter, true);
    return env->NewDirectByteBuffer(iter, 0);
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_nextIterator
  (JNIEnv *env, jobject self, jobject iteratorPtr) {

    UastIterator *iter = (UastIterator*) env->GetDirectBufferAddress(iteratorPtr);

    if (env->ExceptionCheck() == JNI_TRUE) {
      return NULL;
    }
    if (!iter) {
      ThrowException("Could not recover native iterator from UastIterator");
      return NULL;
    }
    memTracker.SetCurrentIterator(iter, false);

    jobject *retNode = (jobject *)UastIteratorNext(iter);
    if (retNode == NULL) {
      // end of the iteration reached
      return NULL;
    }
    return *retNode;
}

JNIEXPORT void JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_disposeIterator
  (JNIEnv *env, jobject self, jobject iteratorPtr) {

    UastIterator *iter = (UastIterator*) env->GetDirectBufferAddress(iteratorPtr);
    if (env->ExceptionCheck() == JNI_TRUE) {
      return;
    }
    if (!iter) {
      ThrowException("Could not recover native iterator from UastIterator");
      return;
    }

    memTracker.DisposeMem(env);
    UastIteratorFree(iter);
}*/

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_ContextExt_root
  (JNIEnv *env, jobject self) {
  ContextExt *p = getHandle<ContextExt>(env, self, "nativeContex");
  return p->RootNode();
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_ContextExt_encode
  (JNIEnv *env, jobject self, jobject node, jint fmt) {
  UastFormat format = UAST_BINARY; // TODO: make it arg

  ContextExt *p = getHandle<ContextExt>(env, self, "nativeContex");
  return p->Encode(node, format);
}

JNIEXPORT void JNICALL Java_org_bblfsh_client_ContextExt_dispose
  (JNIEnv *env, jobject self) {
  ContextExt *p = getHandle<ContextExt>(env, self, "nativeContex");
  setHandle<ContextExt>(env, self, 0, "nativeContex");
  delete p;
}


JNIEXPORT jobject JNICALL Java_org_bblfsh_client_NodeExt_load
  (JNIEnv *env, jobject self){
  auto ctx = new Context();
  jobject node = ctx->LoadFrom(self);
  delete(ctx);
  return node;
}


JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_decode
  (JNIEnv *env, jobject self, jobject directBuf) {
  UastFormat format = UAST_BINARY; // TODO: make it arg

  //works only with ByteBuffer.allocateDirect()
  void* buf = env->GetDirectBufferAddress(directBuf);
  if (env->ExceptionCheck() == JNI_TRUE) {
    return NULL;
  }
  jlong len = env->GetDirectBufferCapacity(directBuf);
  if (env->ExceptionCheck() == JNI_TRUE) {
    return NULL;
  }

  //another option is to use
  //GetPrimitiveArrayCritical
  uast::Buffer ubuf(buf, (size_t)(len));
  uast::Context<NodeHandle> *ctx = uast::Decode(ubuf, format);
  //ReleasePrimitiveArrayCritical

  auto p = new ContextExt(ctx);

  jobject jCtxExt = NewJavaObject(env, "org/bblfsh/client/ContextExt", "(J)V", p);
  if (env->ExceptionCheck() == JNI_TRUE || !jCtxExt) {
    jCtxExt = NULL;
    delete(ctx);
    env->ExceptionDescribe();
    throw std::runtime_error("failed to instantiate ContextExt class");
  }

  return jCtxExt;
}

/*
static const char *InitFilter(JNIEnv *env, jobject obj, jstring query) {
  memTracker.EnterFilter();

  const char *cstr = AsNativeStr(query);
  if (env->ExceptionCheck() == JNI_TRUE || !cstr) {
    throw std::runtime_error("");
  }
  return cstr;
}

static void FinishFilter(JNIEnv *env) {
  memTracker.DisposeMem(env);
  memTracker.ExitFilter();
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_filter
  (JNIEnv *env, jobject self, jobject obj, jstring query) {

  memTracker.EnterFilter();
  Nodes *nodes = NULL;
  jobject nodeList = NULL;

  try {
    jobject *node = &obj;
    nodeList = NewJavaObject(env, CLS_MUTLIST, "()V");
    if (env->ExceptionCheck() == JNI_TRUE || !nodeList) {
      nodeList = NULL;
      throw std::runtime_error("");
    }

    const char *cstr = AsNativeStr(query);
    if (env->ExceptionCheck() == JNI_TRUE || !cstr) {
      throw std::runtime_error("");
    }

    nodes = UastFilter(ctx, node, cstr);
    if (!nodes) {
      ThrowException(LastError());
      throw std::runtime_error("");
    }

    for (int i = 0; i < NodesSize(nodes); i++) {
      jobject *n = (jobject *) NodeAt(nodes, i);
      if (!n) {
        ThrowException("Unable to access a node");
        throw std::runtime_error("");
      }

      ObjectMethod(env, "$plus$eq", METHOD_LIST_PLUSEQ, CLS_MUTLIST, &nodeList, *n);
      if (env->ExceptionCheck() == JNI_TRUE) {
        throw std::runtime_error("");
      }
    }
  } catch (std::runtime_error&) {}

  if (nodes)
    NodesFree(nodes);

  jobject immList = NULL;

  if (nodeList) {
    // Convert to immutable list
    immList = ObjectMethod(env, "toList", METHOD_MUTLIST_TOIMMLIST, CLS_LIST, &nodeList);
  }

  FinishFilter(env);
  return immList;
}

JNIEXPORT jboolean JNICALL Java_org_bblfsh_client_libuast_Libuast_filterBool
  (JNIEnv *env, jobject self, jobject obj, jstring query) {

  jobject *node = &obj;
  jboolean ret = false;

  try {
    auto cstr = InitFilter(env, obj, query);
    bool ok;
    ret = (jboolean)UastFilterBool(ctx, node, cstr, &ok);
    if (!ok) {
      ThrowException(LastError());
      throw std::runtime_error("");
    }
  } catch (std::runtime_error&) {}

  FinishFilter(env);
  return ret;
}

JNIEXPORT jdouble JNICALL Java_org_bblfsh_client_libuast_Libuast_filterNumber
  (JNIEnv *env, jobject self, jobject obj, jstring query) {

  jobject *node = &obj;
  jdouble ret = false;

  try {
    auto cstr = InitFilter(env, obj, query);
    bool ok;
    ret = (jdouble)UastFilterNumber(ctx, node, cstr, &ok);
    if (!ok) {
      ThrowException(LastError());
      throw std::runtime_error("");
    }
  } catch (std::runtime_error&) {}

  FinishFilter(env);
  return ret;
}

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_filterString
  (JNIEnv *env, jobject self, jobject obj, jstring query) {

  jobject *node = &obj;
  const char *retStr = NULL;

  try {
    auto cstr = InitFilter(env, obj, query);
    retStr = UastFilterString(ctx, node, cstr);
    if (retStr == NULL) {
      ThrowException(LastError());
      throw std::runtime_error("");
    }
  } catch (std::runtime_error&) {}

  FinishFilter(env);
  return env->NewStringUTF(retStr);
}
*/

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  jvm = vm;
  //ctx = CreateUast();
  return JNI_VERSION_1_8;
}
