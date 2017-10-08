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
// TODO: apply stuff on https://www.ibm.com/developerworks/library/j-jni/index.html#exceptions

static const char *SIGN_STR = "Ljava/lang/String;";
static const char *SIGN_SEQ = "Lscala/collection/Seq;";

static const char *CLS_NODE = "gopkg/in/bblfsh/sdk/v1/uast/generated/Node";
static const char *CLS_VECTOR = "scala/collection/immutable/Vector";

static JNIEnv *env;
static Uast *ctx;

// Helpers
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

    // Note: printing the type from Scala to find the type needed for
    // GetFieldID third argument using getClass.getName returns Vector but
    // we've to use Seq. To find the right type to use do this from Scala:
    // (instance).getClass.getDeclaredField("fieldName")
    jfieldID childVecId = (*env)->GetFieldID(env, cls, property, SIGN_SEQ);
    if ((*env)->ExceptionOccurred(env) || !childVecId)
        return 0;

    jobject childVector = (*env)->GetObjectField(env, *node, childVecId);
    if ((*env)->ExceptionOccurred(env) || !childVector)
        return 0;

    // get the Vector length; gRPC child container nodes are maped to scala vectors
    jclass vectorCls = (*env)->FindClass(env, CLS_VECTOR);
    if ((*env)->ExceptionOccurred(env) || !vectorCls)
        return 0;

    // Get the size calling the "length" method on the Vector
    jmethodID mLen = (*env)->GetMethodID(env, vectorCls, "length", "()I");
    if ((*env)->ExceptionOccurred(env))
        return 0;

    jint len = (*env)->CallIntMethod(env, childVector, mLen);
    if ((*env)->ExceptionOccurred(env))
        return 0;

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
    ctx = UastNew(iface);

    return JNI_VERSION_1_8;
}

#ifdef __cplusplus
}
#endif
