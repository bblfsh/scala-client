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

jint JNI_OnLoad(JavaVM *vm, void *reserved);

// XXX remove
JNIEXPORT jboolean JNICALL Java_org_bblfsh_client_libuast_Libuast_hasstartposition
  (JNIEnv *, jobject, jobject);

JNIEXPORT jboolean JNICALL Java_org_bblfsh_client_libuast_Libuast_hasendposition
  (JNIEnv *, jobject, jobject);

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_propertykeyat
  (JNIEnv *, jobject, jobject, jint);

JNIEXPORT jstring JNICALL Java_org_bblfsh_client_libuast_Libuast_propertyvalueat
  (JNIEnv *, jobject, jobject, jint);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_startoffset
  (JNIEnv *, jobject, jobject);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_startline
  (JNIEnv *, jobject, jobject);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_startcol
  (JNIEnv *, jobject, jobject);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_endoffset
  (JNIEnv *, jobject, jobject);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_endline
  (JNIEnv *, jobject, jobject);

JNIEXPORT jint JNICALL Java_org_bblfsh_client_libuast_Libuast_endcol
  (JNIEnv *, jobject, jobject);
#ifdef __cplusplus
}
#endif
#endif
