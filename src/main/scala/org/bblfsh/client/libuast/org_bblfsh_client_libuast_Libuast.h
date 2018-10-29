/* Header for class org_bblfsh_client_libuast_Libuast */
#ifndef _Included_org_bblfsh_client_libuast_Libuast
#define _Included_org_bblfsh_client_libuast_Libuast

#ifdef __cplusplus
extern "C" {
#endif

#include <jni.h>

/* ContextExt */
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_ContextExt_root
  (JNIEnv *env, jobject);

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_ContextExt_encode
  (JNIEnv *env, jobject, jobject, jint);

JNIEXPORT void JNICALL Java_org_bblfsh_client_ContextExt_dispose
  (JNIEnv *env, jobject);


/* NodeExt */
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_NodeExt_load
  (JNIEnv *env, jobject);


/* Libuast */
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_decode
  (JNIEnv *env, jobject, jobject);


/*
 * Class:   org_bblfsh_client_libuast_Libuast
 * Method:  filter
 * Signature: (ILjava/lang/String;)Lscala/collection/immutable/List;
*
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_filter
  (JNIEnv *, jobject, jobject, jstring);

JNIEXPORT jboolean JNICALL Java_org_bblfsh_client_libuast_Libuast_filterBool
  (JNIEnv *, jobject, jobject, jstring);

JNIEXPORT jdouble JNICALL Java_org_bblfsh_client_libuast_Libuast_filterNumber
  (JNIEnv *, jobject, jobject, jstring);

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_filterString
  (JNIEnv *, jobject, jobject, jstring);

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_iterate
  (JNIEnv *env, jobject self, jobject obj, int treeOrder, int iterations);

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_newIterator
  (JNIEnv *, jobject, jobject, int);

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_nextIterator
  (JNIEnv *, jobject, jobject);

JNIEXPORT void JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_disposeIterator
  (JNIEnv *, jobject, jobject);
*/
jint JNI_OnLoad(JavaVM *vm, void *reserved);

#ifdef __cplusplus
}
#endif
#endif
