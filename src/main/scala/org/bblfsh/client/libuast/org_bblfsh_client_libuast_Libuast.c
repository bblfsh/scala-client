#include "org_bblfsh_client_libuast_Libuast.h"

#include "uast.h"
#include "jni_md.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h> // XXX remove

#ifdef __cplusplus
extern "C" {
#endif

// TODO: adapt to coding conventions
// TODO: check https://www.ibm.com/developerworks/library/j-jni/index.html

// Type signatures. To get the signature of all methods from a class do:
// javap -s -p SomeClass.class
static const char *SIGN_OBJECT = "Ljava/lang/Object;";
static const char *SIGN_STR = "Ljava/lang/String;";
static const char *SIGN_SEQ = "Lscala/collection/Seq;";
static const char *SIGN_MAP = "Lscala/collection/immutable/Map;";

static const char *CLS_NODE = "gopkg/in/bblfsh/sdk/v1/uast/generated/Node";
static const char *CLS_SEQ = "scala/collection/Seq";
static const char *CLS_MAP = "scala/collection/Map";
static const char *CLS_TUPLE2 = "scala/Tuple2";

static JNIEnv *env;
static Uast *ctx;
static const jclass *NODE_JCLASS;

// Helpers
jint IntMethod(const char *name, const char *signature, const jclass cls,
               const jobject *object)
{
    jmethodID mId = (*env)->GetMethodID(env, cls, name, signature);
    if ((*env)->ExceptionOccurred(env))
        return 0;

    jint res = (*env)->CallIntMethod(env, *object, mId);
    if ((*env)->ExceptionOccurred(env))
        return 0;

    return res;
}

jobject ApplyMethod(const jclass cls, const jobject object, int index)
{
    jmethodID mApply = (*env)->GetMethodID(env, cls, "apply", "(I)Ljava/lang/Object;");
    if ((*env)->ExceptionOccurred(env))
        return NULL;

    jobject res = (*env)->CallObjectMethod(env, object, mApply, index);
    if ((*env)->ExceptionOccurred(env))
        return NULL;

    return res;
}

jobject GetNodeSeqField(const jobject *node, const char *property)
{
    jclass cls = (*env)->FindClass(env, CLS_NODE);
    if ((*env)->ExceptionOccurred(env) || !cls)
        return NULL;

    // Note: printing the type from Scala to find the type needed for
    // GetFieldID third argument using getClass.getName returns Vector but this
    // only works with the Seq trait. To find the right type to use do this
    // from Scala: (instance).getClass.getDeclaredField("fieldName")
    jfieldID childVecId = (*env)->GetFieldID(env, cls, property, SIGN_SEQ);
    if ((*env)->ExceptionOccurred(env) || !childVecId)
        return NULL;

    jobject childSeq = (*env)->GetObjectField(env, *node, childVecId);
    if ((*env)->ExceptionOccurred(env) || !childSeq)
        return NULL;

    return childSeq;
}

// XXX factorize with above
jobject GetNodeProperties(const jobject *node)
{
    jclass cls = (*env)->FindClass(env, CLS_NODE);
    if ((*env)->ExceptionOccurred(env) || !cls)
        return NULL;

    jfieldID propsMapId = (*env)->GetFieldID(env, cls, "properties", SIGN_MAP);
    if ((*env)->ExceptionOccurred(env) || !propsMapId)
        return NULL;

    jobject propsMap = (*env)->GetObjectField(env, *node, propsMapId);
    if ((*env)->ExceptionOccurred(env) || !propsMap)
        return NULL;

    return propsMap;
}

static const char *ReadStr(const jobject *node, const char *property)
{
    jclass cls = (*env)->FindClass(env, CLS_NODE);
    if ((*env)->ExceptionOccurred(env) || !cls)
        return NULL;

    jfieldID fid = (*env)->GetFieldID(env, cls, property, SIGN_STR);
    if ((*env)->ExceptionOccurred(env) || !fid)
        return NULL;

    jstring jvstr = (jstring)(*env)->GetObjectField(env, *node, fid);
    if ((*env)->ExceptionOccurred(env) || !jvstr)
        return NULL;

    const char *cstr = (*env)->GetStringUTFChars(env, jvstr, 0);
    if ((*env)->ExceptionOccurred(env) || !cstr)
        return NULL;

    // str must be copied to deref the java string befeore return
    const char *cstrdup = strdup(cstr);
    if ((*env)->ExceptionOccurred(env) || !cstrdup)
        return NULL;

    (*env)->ReleaseStringUTFChars(env, jvstr, cstr);
    if ((*env)->ExceptionOccurred(env))
        return NULL;

    return cstrdup;
}

static int ReadLen(const jobject *node, const char *property)
{
    jclass cls = (*env)->FindClass(env, CLS_NODE);
    if ((*env)->ExceptionOccurred(env) || !cls)
        return 0;

    jobject childSeq = GetNodeSeqField(node, property);
    if ((*env)->ExceptionOccurred(env) || !cls)
        return 0;

    // get the Seq length; gRPC child container nodes are maped to scala Seqs
    jclass seqCls = (*env)->FindClass(env, CLS_SEQ);
    if ((*env)->ExceptionOccurred(env) || !seqCls)
        return 0;

    return (int)IntMethod("length", "()I", seqCls, &childSeq);
}

// Node interface functions
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
    jobject childSeq = GetNodeSeqField(node, "children");
    if ((*env)->ExceptionOccurred(env))
        return NULL;

    // Call the apply(i) method, jni style
    jclass seqCls = (*env)->FindClass(env, CLS_SEQ);
    if ((*env)->ExceptionOccurred(env) || !seqCls)
        return NULL;

    jobject child = ApplyMethod(seqCls, childSeq, index);
    if ((*env)->ExceptionOccurred(env))
        return NULL;

    jobject *childCopy = malloc(sizeof(jobject));
    memcpy(childCopy, &child, sizeof(child));
    return childCopy;
}

static int PropertiesSize(const void *data)
{
    jobject *node = (jobject *)data;
    // XXX error control
    jobject propsMap = GetNodeProperties(node);
    jclass mapCls = (*env)->FindClass(env, CLS_MAP);
    if ((*env)->ExceptionOccurred(env) || !mapCls)
        return 0;

    return (int)IntMethod("size", "()I", mapCls, &propsMap);
}

// XXX error control, this should reshot the exception
static const char *PropertyAt(const void *data, int index)
{
    jobject *node = (jobject *)data;
    jobject propsMap = GetNodeProperties(node);
    if ((*env)->ExceptionOccurred(env) || !mapCls)
        return NULL;

    jclass mapCls = (*env)->FindClass(env, CLS_MAP);
    if ((*env)->ExceptionOccurred(env) || !mapCls)
        return NULL;

    // Convert to List
    jmethodID mToListId = (*env)->GetMethodID(env, mapCls, "toList",
                                            "()Lscala/collection/immutable/List;");
    if ((*env)->ExceptionOccurred(env) || !mToListId)
        return NULL;

    jobject list = (*env)->CallObjectMethod(env, propsMap, mToListId);

    // Call apply on the list to get the index tuple
    jclass listCls = (*env)->FindClass(env, CLS_SEQ);
    if ((*env)->ExceptionOccurred(env) || !listCls)
        return NULL;

    jobject kvTuple = ApplyMethod(listCls, list, index);
    if ((*env)->ExceptionOccurred(env) || !tupleCls)
        return NULL;

    // Get the "_1" field and convert to char*
    jclass tupleCls = (*env)->FindClass(env, CLS_TUPLE2);
    if ((*env)->ExceptionOccurred(env) || !tupleCls)
        return NULL;

    jfieldID firstId = (*env)->GetFieldID(env, tupleCls, "_1", SIGN_OBJECT);
    if ((*env)->ExceptionOccurred(env) || !firstId)
        return NULL;

    jobject key = (*env)->GetObjectField(env, kvTuple, firstId);
    if ((*env)->ExceptionOccurred(env) || !key)
        return NULL;

    const char *cstr = (*env)->GetStringUTFChars(env, key, 0);
    if ((*env)->ExceptionOccurred(env) || !cstr)
        return NULL;

    return cstr;
}

// Exported Java functions
// TODO, change the jint for a jobject* (the List returned)
JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_filter
  (JNIEnv *env, jobject self, jint i, jstring s) {

    return i;
}

// for testing
JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_readfield
  (JNIEnv *env, jobject self, jobject node, jstring field) {

    const char *cfield = (*env)->GetStringUTFChars(env, field, 0);
    const char *cvalue = ReadStr(&node, cfield);
    (*env)->ReleaseStringUTFChars(env, field, cfield);

    return (*env)->NewStringUTF(env, cvalue);
}

// for testing, rename as testing_*
JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_readlen
  (JNIEnv *env, jobject self, jobject node, jstring field) {

    const char *cfield = (*env)->GetStringUTFChars(env, field, 0);
    return (jint)ReadLen(&node, cfield);
}

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_InternalType
  (JNIEnv *env, jobject self, jobject node) {
      const char *cvalue = InternalType(&node);
      return (*env)->NewStringUTF(env, cvalue);
}

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_Token
  (JNIEnv *env, jobject self, jobject node) {
      const char *cvalue = Token(&node);
      return (*env)->NewStringUTF(env, cvalue);
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_ChildrenSize
  (JNIEnv *env, jobject self, jobject node) {
      int csize = ChildrenSize(&node);
      return (jint)csize;
}

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_ChildAt
  (JNIEnv *env, jobject self, jobject node, jint index) {
      jobject *child = (jobject *)ChildAt(&node, 0);
      return *child;
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_RolesSize
  (JNIEnv *env, jobject self, jobject node) {
      int rsize = RolesSize(&node);
      return (jint)rsize;
}

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_PropertiesSize
  (JNIEnv *env, jobject self, jobject node) {
      int psize = PropertiesSize(&node);
      return (jint)psize;
}

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_PropertyAt
  (JNIEnv *env, jobject self, jobject node, jint index) {

    const char *prop = PropertyAt(&node, index);
    return (*env)->NewStringUTF(env, prop);
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    if ((*vm)->GetEnv(vm, (void **) &env, JNI_VERSION_1_8) != JNI_OK) {
        return JNI_ERR;
    }

    NodeIface iface = {
        .InternalType = InternalType,
        .Token = Token,
        .ChildrenSize = ChildrenSize,
        .ChildAt = ChildAt,
        .RolesSize = RolesSize,
        .PropertiesSize = PropertiesSize,
        .PropertyAt = PropertyAt
    };

    ctx = UastNew(iface);

    return JNI_VERSION_1_8;
}

#ifdef __cplusplus
}
#endif
