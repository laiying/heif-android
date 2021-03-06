/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_strod_heif_HeifNative */

#ifndef _Included_com_strod_heif_HeifNative
#define _Included_com_strod_heif_HeifNative
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     com_strod_heif_HeifNative
 * Method:    encodeBitmap
 * Signature: ([BIILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_strod_heif_HeifNative_encodeBitmap
  (JNIEnv *, jclass, jbyteArray, jint, jint, jstring);

/*
 * Class:     com_strod_heif_HeifNative
 * Method:    encodeYUV
 * Signature: ([BIILjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_strod_heif_HeifNative_encodeYUV
  (JNIEnv *, jclass, jbyteArray, jint, jint, jstring);

/*
 * Class:     com_strod_heif_HeifNative
 * Method:    decodeHeif2RGBA
 * Signature: (Lcom/wanghonglin/libheif/HeifSize;Ljava/lang/String;)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_strod_heif_HeifNative_decodeHeif2RGBA
  (JNIEnv *, jclass, jobject, jstring);

JNIEXPORT jboolean JNICALL Java_com_strod_heif_HeifNative_heif2jpg
        (JNIEnv *, jclass, jstring, jstring);

#ifdef __cplusplus
}
#endif
#endif
