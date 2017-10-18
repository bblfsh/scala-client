#ifdef __cplusplus
extern "C" {
#endif

#include "org_bblfsh_client_libuast_Libuast.h"
#include "utils.h"
#include "nodeiface.h"

#include "uast.h"

JavaVM *jvm;
static Uast *ctx;
extern NodeIface iface;

//// Exported Java functions ////

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_filter
  (JNIEnv *env, jobject self, jobject obj, jstring query) {
  Nodes *nodes = NULL;
  jobject nodeList = NULL;

  if ((*env)->MonitorEnter(env, self) != JNI_OK)
    goto exit;

  jobject *node = &obj;
  nodeList = NewJavaObject(CLS_MUTLIST, "()V");
  if ((*env)->ExceptionOccurred(env) || !nodeList) {
    nodeList = NULL;
    goto exit;
  }

  const char *cstr = AsNativeStr(query);
  if ((*env)->ExceptionOccurred(env) || !cstr)
    goto exit;

  nodes = UastFilter(ctx, node, cstr);
  if (!nodes)
    goto exit;

  int len = NodesSize(nodes);

  // Instantiate a MutableList and append the elements
  if ((*env)->ExceptionOccurred(env) || !nodeList)
    goto exit;

  int i;
  for (i= 0; i < len; i++) {
    jobject *n = (jobject *) NodeAt(nodes, i);
    if (!n)
      continue;

    ObjectMethod("$plus$eq", SIGN_PLUSEQ, CLS_MUTLIST, nodeList, *n);
    if ((*env)->ExceptionOccurred(env))
      goto exit;
  }

exit:
  if (nodes)
    NodesFree(nodes);

  jobject immList = NULL;

  if (nodeList) {
    // Convert to immutable list
    immList = ObjectMethod("toList", SIGN_TOIMMLIST, CLS_LIST, nodeList);
  }

  (*env)->MonitorExit(env, self);

  return immList;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved) {
  jvm = vm;
  ctx = CreateUast();
  return JNI_VERSION_1_8;
}

// XXX remove
JNIEXPORT jboolean JNICALL Java_org_bblfsh_client_libuast_Libuast_hasstartposition
  (JNIEnv *env, jobject self, jobject obj) {
  return _HasPosition(&obj, "startPosition");
}

JNIEXPORT jboolean JNICALL Java_org_bblfsh_client_libuast_Libuast_hasendposition
  (JNIEnv *env, jobject self, jobject obj) {
  return _HasPosition(&obj, "endPosition");
}

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_propertykeyat
  (JNIEnv *env, jobject self, jobject obj, jint index) {
  return (*env)->NewStringUTF(env, PropertyKeyAt(&obj, (int)index));
}

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_propertyvalueat
  (JNIEnv *env, jobject self, jobject obj, jint index) {
  return (*env)->NewStringUTF(env, PropertyValueAt(&obj, (int)index));
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_startoffset
  (JNIEnv *env, jobject self, jobject obj) {
  return (jint)StartOffset(&obj);
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_startline
  (JNIEnv *env, jobject self, jobject obj) {
  return (jint)StartLine(&obj);
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_startcol
  (JNIEnv *env, jobject self, jobject obj) {
  return (jint)StartCol(&obj);
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_endoffset
  (JNIEnv *env, jobject self, jobject obj) {
  return (jint)EndOffset(&obj);
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_endline
  (JNIEnv *env, jobject self, jobject obj) {
  return (jint)EndLine(&obj);
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_endcol
  (JNIEnv *env, jobject self, jobject obj) {
  return (jint)EndCol(&obj);
}
#ifdef __cplusplus
}
#endif
