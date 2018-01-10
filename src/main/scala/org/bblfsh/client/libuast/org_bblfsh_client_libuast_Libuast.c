#include "org_bblfsh_client_libuast_Libuast.h"
#include "jni_utils.h"
#include "nodeiface.h"
#include "objtrack.h"

#include <stdio.h> // XXX

#include "uast.h"

// XXX remove
JavaVM *jvm;
static Uast *ctx;

//// Exported Java functions ////

jobject *copyReferencedNode(JNIEnv *env, jobject node) {
  // jobject itself is a opaque struct with a pointer and the pointed obj is deallocated
  // at the end so we need to increase the global ref and copy the returned jobject
  // XXX Delete Ref after usage,
  jobject ref = (*env)->NewGlobalRef(env, node);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE || !ref) {
    return NULL;
  }

  jobject *nodeptr = malloc(sizeof(jobject));
  memcpy(nodeptr, &ref, sizeof(jobject));
  // XXX needs a specific version for iterators so allocs doesn't get mixer with filter's
  // ones
  trackObject((void *)nodeptr);
  return nodeptr;
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_iterate
  (JNIEnv *env, jobject self, jobject obj, jint treeOrder, jint iterations) {

    UastIterator *iter = UastIteratorNew(ctx, &obj, (TreeOrder)treeOrder);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      return NULL;
    }
    if (!iter) {
      ThrowException(LastError());
      return 0;
    }

    jobject *next = NULL;

    for (int i = 0; i <= iterations; i++) {
      next = (jobject *)UastIteratorNext(iter);
      if (next == NULL) {
        break; // Return Optional Null instead
      }
    }

    UastIteratorFree(iter);
    return *next;
}

// 00024 = "$" in .class files == Inner class reference
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_newIterator
  (JNIEnv *env, jobject self, jobject obj, int treeOrder) {

    jobject *nodeptr = copyReferencedNode(env, obj);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE || !nodeptr) {
      return NULL;
    }

    UastIterator *iter = UastIteratorNew(ctx, nodeptr, (TreeOrder)treeOrder);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      return NULL;
    }
    if (!iter) {
      ThrowException(LastError());
      return 0;
    }

    return (*env)->NewDirectByteBuffer(env, iter, 0);
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_nextIterator
  (JNIEnv *env, jobject self, jobject iteratorPtr) {

    UastIterator *iter = (UastIterator*) (*env)->GetDirectBufferAddress(env, iteratorPtr);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      return NULL;
    }
    if (!iter) {
      ThrowException("Could not recover native iterator from UastIterator");
      return NULL;
    }

    jobject *retNode = (jobject *)UastIteratorNext(iter);
    if (!retNode) printf("XXX retNode is null\n");
    return *retNode;
}

JNIEXPORT void JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_disposeIterator
  (JNIEnv *env, jobject self, jobject iteratorPtr) {

    printf("XXX disposeIterator\n");
    UastIterator *iter = (UastIterator*) (*env)->GetDirectBufferAddress(env, iteratorPtr);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      return NULL;
    }
    if (!iter) {
      ThrowException("Could not recover native iterator from UastIterator");
      return;
    }

    UastIteratorFree(iter);
    // XXX re-enable with specific version for iterators
    /*freeObjects();*/
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_filter
  (JNIEnv *env, jobject self, jobject obj, jstring query) {
  Nodes *nodes = NULL;
  jobject nodeList = NULL;

  jobject *node = &obj;
  nodeList = NewJavaObject(env, CLS_MUTLIST, "()V");
  if ((*env)->ExceptionCheck(env) == JNI_TRUE || !nodeList) {
    nodeList = NULL;
    goto exit;
  }

  const char *cstr = AsNativeStr(query);
  if ((*env)->ExceptionCheck(env) == JNI_TRUE || !cstr) {
    goto exit;
  }

  nodes = UastFilter(ctx, node, cstr);
  if (!nodes) {
    ThrowException(LastError());
    goto exit;
  }

  for (int i = 0; i < NodesSize(nodes); i++) {
    jobject *n = (jobject *) NodeAt(nodes, i);
    if (!n) {
      ThrowException("Unable to access a node");
      goto exit;
    }

    ObjectMethod(env, "$plus$eq", METHOD_LIST_PLUSEQ, CLS_MUTLIST, &nodeList, *n);
    if ((*env)->ExceptionCheck(env) == JNI_TRUE) {
      goto exit;
    }
  }

exit:
  freeObjects();

  if (nodes)
    NodesFree(nodes);

  jobject immList = NULL;

  if (nodeList) {
    // Convert to immutable list
    immList = ObjectMethod(env, "toList", METHOD_MUTLIST_TOIMMLIST, CLS_LIST, &nodeList);
  }

  return immList;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  jvm = vm;
  ctx = CreateUast();
  return JNI_VERSION_1_8;
}
