/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_bblfsh_client_v2_Context */

#ifndef _Included_org_bblfsh_client_v2_Context
#define _Included_org_bblfsh_client_v2_Context
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     org_bblfsh_client_v2_Context
 * Method:    root
 * Signature: ()Lorg/bblfsh/client/v2/JNode;
 */
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_Context_root
  (JNIEnv *, jobject);

/*
 * Class:     org_bblfsh_client_v2_Context
 * Method:    filter
 * Signature: (Ljava/lang/String;Lorg/bblfsh/client/v2/JNode;)Lorg/bblfsh/client/v2/libuast/Libuast/UastIter;
 */
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_Context_filter
  (JNIEnv *, jobject, jstring, jobject);

/*
 * Class:     org_bblfsh_client_v2_Context
 * Method:    nativeEncode
 * Signature: (Lorg/bblfsh/client/v2/JNode;I)Ljava/nio/ByteBuffer;
 */
JNIEXPORT jobject JNICALL Java_org_bblfsh_client_v2_Context_nativeEncode
  (JNIEnv *, jobject, jobject, jint);

/*
 * Class:     org_bblfsh_client_v2_Context
 * Method:    dispose
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_org_bblfsh_client_v2_Context_dispose
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
