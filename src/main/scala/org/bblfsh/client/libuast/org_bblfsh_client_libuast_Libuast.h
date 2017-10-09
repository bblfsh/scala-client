#include <jni.h>
/* Header for class org_bblfsh_client_libuast_Libuast */

#ifndef _Included_org_bblfsh_client_libuast_Libuast
#define _Included_org_bblfsh_client_libuast_Libuast
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_bblfsh_client_libuast_Libuast
 * Method:    filter
 * Signature: (ILjava/lang/String;)Lscala/collection/immutable/List;
 */
// XXX jobject return
JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_filter
  (JNIEnv *, jobject, jint, jstring);

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_readfield
  (JNIEnv *, jobject, jobject, jstring);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_readlen
  (JNIEnv *, jobject, jobject, jstring);

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_InternalType
  (JNIEnv *env, jobject self, jobject node);

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_Token
  (JNIEnv *env, jobject self, jobject node);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_ChildrenSize
  (JNIEnv *env, jobject self, jobject node);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_RolesSize
  (JNIEnv *env, jobject self, jobject node);

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_ChildAt
  (JNIEnv *env, jobject self, jobject node, jint index);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_PropertiesSize
  (JNIEnv *env, jobject self, jobject node);

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_PropertyAt
  (JNIEnv *env, jobject self, jobject node, jint index);
#ifdef __cplusplus
}
#endif
#endif
