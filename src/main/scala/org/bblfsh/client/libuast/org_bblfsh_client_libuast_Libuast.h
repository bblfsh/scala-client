/* Header for class org_bblfsh_client_libuast_Libuast */

#ifndef _Included_org_bblfsh_client_libuast_Libuast
#define _Included_org_bblfsh_client_libuast_Libuast

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:   org_bblfsh_client_libuast_Libuast
 * Method:  filter
 * Signature: (ILjava/lang/String;)Lscala/collection/immutable/List;
*/
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_filter
  (JNIEnv *, jobject, jobject, jstring);

JNIEXPORT jlong JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_newIterator
  (JNIEnv *, jobject, jobject, int);

JNIEXPORT jobject JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_nextIterator
  (JNIEnv *, jobject, jlong);

JNIEXPORT void JNICALL Java_org_bblfsh_client_libuast_Libuast_00024UastIterator_disposeIterator
  (JNIEnv *, jobject, jlong);

jint JNI_OnLoad(JavaVM *vm, void *reserved);
#ifdef __cplusplus
}
#endif
#endif
