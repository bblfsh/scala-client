#include "org_bblfsh_client_libuast_Libuast.h"

#include "uast.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// TODO: coding conventions
// TODO: move the helper functions to utils.c/h and the binding to binding.c/h
// TODO: include guards

// Type signatures; to get the signature of all methods from a class do:
// javap -s -p SomeClass.class
// To create a Java version of this module from the same codebase, #ifdefs should
// be sprinkled here to get the equivalent Java types.
static const char *SIGN_OBJECT = "Ljava/lang/Object;";
static const char *SIGN_STR = "Ljava/lang/String;";
static const char *SIGN_SEQ = "Lscala/collection/Seq;";
static const char *SIGN_MAP = "Lscala/collection/immutable/Map;";

// Method signatures. Same as above: use javap to get them from a .class
static const char *SIGN_APPLY = "(I)Ljava/lang/Object;";
static const char *SIGN_TOLIST = "()Lscala/collection/immutable/List;";
static const char *SIGN_TOIMMLIST = "()Lscala/collection/immutable/List;";
static const char *SIGN_LISTINIT = "()V";
static const char *SIGN_APPEND = "(Ljava/lang/Object;)V";
static const char *SIGN_PLUSEQ = "(Ljava/lang/Object;)Lscala/collection/mutable/MutableList;";

// Class fully qualified names
static const char *CLS_NODE = "gopkg/in/bblfsh/sdk/v1/uast/generated/Node";
static const char *CLS_ROLE = "gopkg/in/bblfsh/sdk/v1/uast/generated/Role";
static const char *CLS_SEQ = "scala/collection/Seq";
static const char *CLS_MAP = "scala/collection/Map";
static const char *CLS_TUPLE2 = "scala/Tuple2";
static const char *CLS_LIST = "scala/collection/immutable/List";
static const char *CLS_MUTLIST = "scala/collection/mutable/MutableList";

static JNIEnv *env;
static Uast *ctx;

//// Helpers ////

jobject *ToObjectPtr(jobject *object)
{
  jobject *copy = malloc(sizeof(jobject));
  memcpy(copy, object, sizeof(jobject));
  return copy;
}

const char *AsNativeStr(jstring jstr)
{
  const char *tmp = (*env)->GetStringUTFChars(env, jstr, 0);
  if ((*env)->ExceptionOccurred(env) || !tmp)
    return NULL;

  // str must be copied to deref the java string before return
  const char *cstr = strdup(tmp);

  (*env)->ReleaseStringUTFChars(env, jstr, tmp);
  if ((*env)->ExceptionOccurred(env))
    return NULL;

  return cstr;
}

jint IntMethod(const char *method, const char *signature, const char *className,
         const jobject *object)
{
  jclass cls = (*env)->FindClass(env, className);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return 0;

  jmethodID mId = (*env)->GetMethodID(env, cls, method, signature);
  if ((*env)->ExceptionOccurred(env))
    return 0;

  jint res = (*env)->CallIntMethod(env, *object, mId);
  if ((*env)->ExceptionOccurred(env))
    return 0;

  return res;
}

jobject ObjectMethod(const char *method, const char *signature, const char *typeName,
           const jobject object, ...)
{
  jclass cls = (*env)->FindClass(env, typeName);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return NULL;

  jmethodID mId = (*env)->GetMethodID(env, cls, method, signature);
  if ((*env)->ExceptionOccurred(env) || !mId)
    return NULL;

  va_list varargs;
  va_start(varargs, object);
  jobject res = (*env)->CallObjectMethodV(env, object, mId, varargs);
  va_end(varargs);

  return res;
}

jobject ObjectField(const char *typeName, const jobject *obj, const char *field,
          const char *signature)
{
  jclass cls = (*env)->FindClass(env, typeName);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return NULL;

  // Note: printing the type from Scala to find the type needed for
  // GetFieldID third argument using getClass.getName returns Vector but this
  // only works with the Seq trait. To find the right type to use do this
  // from Scala: (instance).getClass.getDeclaredField("fieldName")
  jfieldID valueId = (*env)->GetFieldID(env, cls, field, signature);
  if ((*env)->ExceptionOccurred(env) || !valueId)
    return NULL;

  jobject value = (*env)->GetObjectField(env, *obj, valueId);
  if ((*env)->ExceptionOccurred(env) || !value)
    return NULL;

  return value;
}

jint IntField(const char *typeName, const jobject *obj, const char *field)
{
  jclass cls = (*env)->FindClass(env, typeName);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return 0;

  jfieldID valueId = (*env)->GetFieldID(env, cls, field, "I");
  if ((*env)->ExceptionOccurred(env) || !valueId)
    return 0;

  jint value = (*env)->GetIntField(env, *obj, valueId);
  if ((*env)->ExceptionOccurred(env))
    return 0;

  return value;
}

jobject NewJavaObject(const char *className, const char *initSign, ...)
{
  jclass cls = (*env)->FindClass(env, className);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return NULL;

  jmethodID initId = (*env)->GetMethodID(env, cls, "<init>", initSign);
  if ((*env)->ExceptionOccurred(env) || !initId)
    return NULL;

  va_list varargs;
  va_start(varargs, initSign);
  jobject instance = (*env)->NewObjectV(env, cls, initId, varargs);
  va_end(varargs);
  if ((*env)->ExceptionOccurred(env) || !instance)
    return NULL;

  return instance;
}

static const char *ReadStr(const jobject *node, const char *property)
{
  jclass cls = (*env)->FindClass(env, CLS_NODE);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return NULL;

  jstring jvstr = (jstring)ObjectField(CLS_NODE, node, property, SIGN_STR);
  if ((*env)->ExceptionOccurred(env) || !jvstr)
    return NULL;

  return AsNativeStr(jvstr);
}

static int ReadLen(const jobject *node, const char *property)
{
  jclass cls = (*env)->FindClass(env, CLS_NODE);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return 0;

  jobject childSeq = ObjectField(CLS_NODE, node, property, SIGN_SEQ);
  if ((*env)->ExceptionOccurred(env) || !cls)
    return 0;

  return (int)IntMethod("length", "()I", CLS_SEQ, &childSeq);
}

//// Node interface functions ////

static const char *InternalType(const void *node)
{
  return ReadStr((const jobject*)node, "internalType");
}

static const char *Token(const void *node)
{
  return ReadStr((jobject*)node, "token");
}

static int ChildrenSize(const void *node)
{
  return ReadLen((jobject*)node, "children");
}

static int RolesSize(const void *node)
{
  return ReadLen((jobject*)node, "roles");
}

void *ChildAt(const void *data, int index)
{
  jobject *node = (jobject *)data;
  jobject childSeq = ObjectField(CLS_NODE, node, "children", SIGN_SEQ);
  if ((*env)->ExceptionOccurred(env))
    return NULL;

  jobject child = ObjectMethod("apply", SIGN_APPLY, CLS_SEQ, childSeq, index);
  if ((*env)->ExceptionOccurred(env) || !child)
    return NULL;

  return ToObjectPtr(&child);
}

static int PropertiesSize(const void *data)
{
  jobject *node = (jobject *)data;
  jobject propsMap = ObjectField(CLS_NODE, node, "properties", SIGN_MAP);
  if ((*env)->ExceptionOccurred(env) || !propsMap)
    return 0;

  return (int)IntMethod("size", "()I", CLS_MAP, &propsMap);
}

static const char *PropertyAt(const void *data, int index)
{
  jobject *node = (jobject *)data;
  jobject propsMap = ObjectField(CLS_NODE, node, "properties", SIGN_MAP);
  if ((*env)->ExceptionOccurred(env) || !propsMap)
    return NULL;

  // Convert to List
  jobject list = ObjectMethod("toList", SIGN_TOLIST, CLS_MAP, propsMap);
  if ((*env)->ExceptionOccurred(env) || !list)
    return NULL;

  // Get the key/value tuple at the "index" position. The tuple is given as a Seq.
  jobject kvTuple = ObjectMethod("apply", SIGN_APPLY, CLS_SEQ, list, index);
  if ((*env)->ExceptionOccurred(env) || !kvTuple)
    return NULL;

  // Get the "_1" field and convert to char*
  jobject key = ObjectField(CLS_TUPLE2, &kvTuple, "_1", SIGN_OBJECT);
  if ((*env)->ExceptionOccurred(env) || !key)
    return NULL;

  return AsNativeStr((jstring)key);
}

static uint16_t RoleAt(const void *data, int index)
{
  jobject *node = (jobject *)data;
  jobject roleSeq = ObjectField(CLS_NODE, node, "roles", SIGN_SEQ);
  if ((*env)->ExceptionOccurred(env) || !roleSeq)
    return 0;

  jobject roleObj = ObjectMethod("apply", SIGN_APPLY, CLS_SEQ, roleSeq, index);
  if ((*env)->ExceptionOccurred(env) || !roleObj)
    return 0;

  jint roleNum = IntMethod("value", "()I", CLS_ROLE, &roleObj);
  if ((*env)->ExceptionOccurred(env))
    return 0;

  return (uint16_t)roleNum;
}

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

// for testing
JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_readfield
  (JNIEnv *env, jobject self, jobject node, jstring field)
{

  const char *cfield = AsNativeStr(field);
  if ((*env)->ExceptionOccurred(env))
    return NULL;

  const char *cvalue = ReadStr(&node, cfield);
  return (*env)->NewStringUTF(env, cvalue);
}

// for testing, rename as testing_*
JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_readlen
  (JNIEnv *env, jobject self, jobject node, jstring field)
{

  const char *cstr = AsNativeStr(field);
  if ((*env)->ExceptionOccurred(env))
    return 0;

  return (jint)ReadLen(&node, cstr);
}

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_InternalType
  (JNIEnv *env, jobject self, jobject node)
{
    const char *cvalue = InternalType(&node);
    return (*env)->NewStringUTF(env, cvalue);
}

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_Token
  (JNIEnv *env, jobject self, jobject node)
{
    const char *cvalue = Token(&node);
    return (*env)->NewStringUTF(env, cvalue);
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_ChildrenSize
  (JNIEnv *env, jobject self, jobject node)
{
    int csize = ChildrenSize(&node);
    return (jint)csize;
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_ChildAt
  (JNIEnv *env, jobject self, jobject node, jint index)
{
    jobject *child = (jobject *)ChildAt(&node, 0);
    return *child;
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_RolesSize
  (JNIEnv *env, jobject self, jobject node)
{
    int rsize = RolesSize(&node);
    return (jint)rsize;
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_RoleAt
  (JNIEnv *env, jobject self, jobject node, jint index)
{

  return (jint)RoleAt(&node, index);
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_PropertiesSize
  (JNIEnv *env, jobject self, jobject node)
{
    int psize = PropertiesSize(&node);
    return (jint)psize;
}

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_PropertyAt
  (JNIEnv *env, jobject self, jobject node, jint index)
{

  const char *prop = PropertyAt(&node, index);
  return (*env)->NewStringUTF(env, prop);
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
  if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_8) != JNI_OK) {
    return JNI_ERR;
  }

  ctx = UastNew((NodeIface){
      .InternalType = InternalType,
      .Token = Token,
      .ChildrenSize = ChildrenSize,
      .ChildAt = ChildAt,
      .RolesSize = RolesSize,
      .RoleAt = RoleAt,
      .PropertiesSize = PropertiesSize,
      .PropertyAt = PropertyAt
  });

  return JNI_VERSION_1_8;
}

#ifdef __cplusplus
}
#endif
