#include "org_bblfsh_client_libuast_Libuast.h"

#include "uast.h"
#include "jni_md.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h> // XXX remove

#ifdef __cplusplus
extern "C" {
#endif

// TODO: adapt to coding conventions
// TODO: cache node class, fields, etc

static const char *ACC_STR = "Ljava/lang/String;";
static const char *ACC_VECTOR = "Lscala/collection/immutable/Vector;";
static const char *CLS_VECTOR = "scala/collection/immutable/Vector";

static JNIEnv *env;
static Uast *ctx;

// Helpers
static const char *ReadStr(const jobject *node, const char *property)
{
    jclass cls = (*env)->GetObjectClass(env, *node);
    jfieldID fid = (*env)->GetFieldID(env, cls, property, ACC_STR);
    if (!fid)
        return NULL;

    jstring jvstr = (jstring)(*env)->GetObjectField(env, *node, fid);
    if (!jvstr)
        return NULL;

    const char *cstr = (*env)->GetStringUTFChars(env, jvstr, 0);
    if (!cstr)
        return NULL;

    // str must be copied to deref the java string befeore return
    const char *cstrdup = strdup(cstr);
    if (!cstrdup)
        return NULL;

    (*env)->ReleaseStringUTFChars(env, jvstr, cstr);

    return cstrdup;
}

static int ReadLen(const jobject *node, const char *property)
{
    jclass cls = (*env)->GetObjectClass(env, *node);
    if (!cls)
        return 0;
    jfieldID childVecId = (*env)->GetFieldID(env, cls, property, ACC_VECTOR);
    if (!childVecId)
        return 0;

    jobject childVector = (*env)->GetObjectField(env, *node, childVecId);
    if (!childVector)
        return 0;

    // get the Vector length; gRPC child container nodes are maped to scala vectors
    jclass vectorCls = (*env)->FindClass(env, CLS_VECTOR);
    if (!vectorCls)
        return 0;

    // Get the size calling the "length" method on the Vector
    jmethodID mLen = (*env)->GetMethodID(env, vectorCls, "length", "()I");
    if (!mLen)
        return 0;

    jint len = (*env)->CallIntMethod(env, childVector, mLen);
    return (int)len;
}

// Node interface functions
static const char *InternalType(const void *node)
{
    return ReadStr((jobject*)node, "internalType");
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

// TODO: static void *ChildAt(const void *data, int index);
// TODO: static int PropertiesSize(const void *data)
// TODO: static const char *PropertyAt(const void *data, int index)

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

// for testing
JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_readlen
  (JNIEnv *env, jobject self, jobject node, jstring field) {

    const char *cfield = (*env)->GetStringUTFChars(env, field, 0);
    return (jint)ReadLen(&node, cfield);
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
        .ChildAt = NULL, // XXX
        .RolesSize = RolesSize,
        .PropertiesSize = NULL, // XXX
        .PropertyAt = NULL // XXX
    };

    // FIXME: undefined symbol
    /*ctx = UastNew(iface);*/

    return JNI_VERSION_1_8;
}

#ifdef __cplusplus
}
#endif
