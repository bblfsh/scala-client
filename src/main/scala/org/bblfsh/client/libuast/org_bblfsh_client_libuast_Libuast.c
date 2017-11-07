#ifdef __cplusplus
extern "C" {
#endif

#include "org_bblfsh_client_libuast_Libuast.h"
#include "jni_utils.h"
#include "nodeiface.h"
#include "objtrack.h"

#include "uast.h"

JavaVM *jvm;

static Uast *ctx;
extern NodeIface iface;

//// Exported Java functions ////

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
#ifdef __cplusplus
}
#endif
