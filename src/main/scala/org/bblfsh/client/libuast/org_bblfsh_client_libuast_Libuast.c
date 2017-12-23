#include "org_bblfsh_client_libuast_Libuast.h"
#include "jni_utils.h"
#include "nodeiface.h"
#include "objtrack.h"
#include "stdio.h" // XXX

#include "uast.h"

JavaVM *jvm;

static Uast *ctx;
extern NodeIface iface;

//// Exported Java functions ////

// 00024 = "$" in .class files == Inner class reference
JNIEXPORT jlong JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_newIterator
  (JNIEnv *env, jobject self, jobject obj, int treeOrder) {

    /*void *node = (void *)&obj;*/
    void *node = ToObjectPtr(&obj);
    UastIterator *iter = UastIteratorNew(ctx, node, (TreeOrder)treeOrder);
    printf("XXX scala_c, iter: %ld\n", iter);
    if (!iter) {
      ThrowException(LastError());
      return 0;
    }

    return (jlong) iter;
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_nextIterator
  (JNIEnv *env, jobject self, jlong iteratorPtr) {

    UastIterator *iter = (UastIterator*) iteratorPtr;
    printf("XXX JNI::next CRASH HERE, iter: %ld\n", iter);
    return *((jobject *)UastIteratorNext(iter));
    /*jobject *node = (jobject *)UastIteratorNext(iter);*/
    /*if (!node) {*/
      /*printf("XXX K1\n");*/
      /*ThrowException("Could not get next Node in interation");*/
      /*return NULL;*/
    /*}*/

    /*printf("XXX K2, need to copy the C node to a Java object\n");*/
    /*return node;*/
}

JNIEXPORT void JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_disposeIterator
  (JNIEnv *env, jobject self, jlong iteratorPtr) {

    UastIterator *iter = (UastIterator*) iteratorPtr;
    if (!iter) {
      // leak, but nothing we can do about it
      ThrowException("Could not recover native iterator from UastIterator");
      return;
    }

    UastIteratorFree(iter);
    freeObjects();
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
