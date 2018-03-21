#include "org_bblfsh_client_libuast_Libuast.h"
#include "jni_utils.h"
#include "nodeiface.h"
#include "memtracker.h"

#include <cstring>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include <unordered_map>
#include <vector>

#include "uast.h"

JavaVM *jvm;
static Uast *ctx;
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

//// Exported Java functions ////

// 00024 = "$" in .class files == Inner class reference
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

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  jvm = vm;
  ctx = CreateUast();
  return JNI_VERSION_1_8;
}
