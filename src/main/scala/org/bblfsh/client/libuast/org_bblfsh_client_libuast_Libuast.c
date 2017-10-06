#include "org_bblfsh_client_libuast_Libuast.h"

#include "uast.h"
#include "jni_md.h"

#include <stdint.h>
#include <string.h>
#include <stdio.h> // XXX remove

#ifdef __cplusplus
extern "C" {
#endif

// XXX coding conventions
// XXX cache node class, fields, etc

// XXX remove
static void _getClassName(JNIEnv *env, jobject obj)
{
    jclass cls = (*env)->GetObjectClass(env, obj);
    // First get the class object
    jmethodID mid = (*env)->GetMethodID(env, cls, "getClass", "()Ljava/lang/Class;");
    jobject clsObj = (*env)->CallObjectMethod(env, obj, mid);

    // Now get the class object's class descriptor
    cls = (*env)->GetObjectClass(env, clsObj);

    // Find the getName() method on the class object
    mid = (*env)->GetMethodID(env, cls, "getName", "()Ljava/lang/String;");

    // Call the getName() to get a jstring object back
    jstring strObj = (jstring)(*env)->CallObjectMethod(env, clsObj, mid);

    // Now get the c string from the java jstring object
    const char* str = (*env)->GetStringUTFChars(env, strObj, NULL);

    // Print the class name
    printf("\nCalling class is: %s\n", str);

    // Release the memory pinned char array
    (*env)->ReleaseStringUTFChars(env, strObj, str);
}

static const char *ReadStr(JNIEnv *env, const jobject node, const char *property)
{
    jclass cls = (*env)->GetObjectClass(env, node);
    jfieldID fid = (*env)->GetFieldID(env, cls, property, "Ljava/lang/String;");
    if (!fid)
        return NULL;

    jstring jvstr = (jstring)(*env)->GetObjectField(env, node, fid);
    const char *cstr = (*env)->GetStringUTFChars(env, jvstr, 0);
    // str must be copied to deref the java string befeore return
    const char *cstrdup = strdup(cstr);
    (*env)->ReleaseStringUTFChars(env, jvstr, cstr);

    return cstrdup;
}

static int ReadLen(JNIEnv *env, const jobject node, const char *property)
{
    jclass cls = (*env)->GetObjectClass(env, node);
    jfieldID fid = (*env)->GetFieldID(env, cls, property,
                                      "Lscala.collection.immutable.Vector;");
    jobject children = (*env)->GetObjectField(env, node, fid);
    if (!children)
        return 0;

    // get the Vector length
    // XXX CRASH
    jclass vector_cls = (*env)->GetObjectClass(env, children);
    jclass vector_cls = (*env)->FindClass(env, "scala.collection.immutable.Vector");
    jfieldID lenid = (*env)->GetFieldID(env, vector_cls, "length",
                                      "Lscala.collection.immutable.Vector;");
    return (int)(*env)->GetIntField(env, children, lenid);
}

// Exported Java function
JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_filter
  (JNIEnv *env, jobject self, jint i, jstring s) {

    return i;
}

// for testing
JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_readfield
  (JNIEnv *env, jobject self, jobject node, jstring field) {

    const char *cfield = (*env)->GetStringUTFChars(env, field, 0);
    const char *cvalue = ReadStr(env, node, cfield);
    (*env)->ReleaseStringUTFChars(env, field, cfield);

    return (*env)->NewStringUTF(env, cvalue);
}

// for testing
JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_readlen
  (JNIEnv *env, jobject self, jobject node, jstring field) {

    const char *cfield = (*env)->GetStringUTFChars(env, field, 0);
    return (jint)ReadLen(env, node, cfield);
}

#ifdef __cplusplus
}
#endif
