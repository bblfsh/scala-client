#ifdef __cplusplus
extern "C" {
#endif

#include "org_bblfsh_client_libuast_Libuast.h"
#include "utils.h"
#include "nodeiface.h"

#include "uast.h"

JNIEnv *env;
static Uast *ctx;
extern NodeIface iface;

//// Exported Java functions ////

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_filter
  (JNIEnv *env, jobject self, jobject obj, jstring query)
{
  jobject *node = &obj;
  jobject nodeList = NewJavaObject(CLS_MUTLIST, "()V");

  const char *cstr = AsNativeStr(query);
  Nodes *nodes = UastFilter(ctx, node, cstr);
  if (!nodes)
    return nodeList; // no results, empty list

  int len = NodesSize(nodes);

  // Instantiate a MutableList and append the elements
  if ((*env)->ExceptionOccurred(env) || !nodeList)
    return NULL;

  for (int i= 0; i < len; i++) {
    jobject *n = (jobject *) NodeAt(nodes, i);
    if (!n)
      continue;

    ObjectMethod("$plus$eq", SIGN_PLUSEQ, CLS_MUTLIST, nodeList, *n);
    if ((*env)->ExceptionOccurred(env)) {
      NodesFree(nodes);
      return NULL;
    }
  }

  NodesFree(nodes);

  // Convert to immutable list
  return ObjectMethod("toList", SIGN_TOIMMLIST, CLS_LIST, nodeList);
}


jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
  if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_8) != JNI_OK) {
    return JNI_ERR;
  }

  ctx = CreateUast();
  return JNI_VERSION_1_8;
}

#ifdef __cplusplus
}
#endif
