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
#ifdef __cplusplus
}
#endif
#endif
