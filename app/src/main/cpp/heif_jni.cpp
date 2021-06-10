
#include <jni.h>
#include <libheif/heif.h>
#include <android/log.h>
#include <cstring>
#include <android/bitmap.h>

#include "encoder.h"
#include "encoder_jpeg.h"

#define TAG "libheif"

extern "C"
JNIEXPORT jint JNICALL
Java_com_strod_heif_HeifNative_encodeBitmap(JNIEnv *env, jclass type, jbyteArray bytes_,
                                                     jint width, jint height, jstring outputPath_) {
    jbyte *bytes = env->GetByteArrayElements(bytes_, NULL);
    jsize length = env->GetArrayLength(bytes_);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    heif_image* image;
    heif_image_create(width, height, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, &image);
    heif_image_add_plane(image, heif_channel_interleaved, width, height, 32);

    int stride = 0;
    uint8_t* p = heif_image_get_plane(image, heif_channel_interleaved, &stride);
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "stride of image %d, %dx%d, %d", stride, width, height, length);

    std::memcpy(p, bytes, static_cast<size_t>(length));

    heif_context* ctx = heif_context_alloc();
    heif_encoder* encoder;
    heif_context_get_encoder_for_format(ctx, heif_compression_HEVC, &encoder);
    heif_encoder_set_logging_level(encoder, 4);

    heif_encoding_options* encoding_options = heif_encoding_options_alloc();
    encoding_options->save_alpha_channel = 0; // must be turned off for Android

    heif_error error;
    heif_image_handle* handle;
    error = heif_context_encode_image(ctx, image, encoder, encoding_options, &handle);
    if (error.code != heif_error_Ok) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "encode image error");
    } else {
        int ow = heif_image_handle_get_width(handle);
        int oh = heif_image_handle_get_height(handle);
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "encode image done %dx%d", ow, oh);
    }
    heif_encoder_release(encoder);

    error = heif_context_write_to_file(ctx, outputPath);
    if (error.code != heif_error_Ok) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "write to file failed");
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "write to file success");
    }

    heif_image_handle_release(handle);
    heif_image_release(image);

    heif_context_free(ctx);

    env->ReleaseByteArrayElements(bytes_, bytes, 0);
    env->ReleaseStringUTFChars(outputPath_, outputPath);

    return error.code;
}

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_com_strod_heif_HeifNative_decodeHeif2RGBA(JNIEnv *env, jclass type, jobject outSize,
                                                        jstring srcPath_) {
    const char *srcPath = env->GetStringUTFChars(srcPath_, 0);

    heif_context* ctx = heif_context_alloc();
    if (!ctx) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG,"Could not create context object");
        return NULL;
    }
    heif_error error;
    error = heif_context_read_from_file(ctx, srcPath, nullptr);
    if (error.code != heif_error_Ok) {
       __android_log_print(ANDROID_LOG_DEBUG, TAG, "read_from_file failed code:%d msg:%s path:%s",error.code, error.message, srcPath);
        heif_context_free(ctx);
        return NULL;
    }
    /*int num_images = heif_context_get_number_of_top_level_images(ctx);
    __android_log_print(ANDROID_LOG_DEBUG, TAG,"File contains path:%s %d images", srcPath, num_images);

    if (num_images == 0) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG,"File doesn't contain any images");
        return NULL;
    }*/

    heif_image_handle* handle;
    error = heif_context_get_primary_image_handle(ctx, &handle);
    if (error.code != heif_error_Ok) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "get_primary_image_handle failed code:%d msg:%s path:%s",error.code, error.message, srcPath);
        heif_context_free(ctx);
        return NULL;
    }

    /*int has_alpha = heif_image_handle_has_alpha_channel(handle);
    __android_log_print(ANDROID_LOG_DEBUG, TAG,"path:%s has_alpha: %d", srcPath, has_alpha);
    struct heif_decoding_options* decode_options = heif_decoding_options_alloc();*/

    /*int bit_depth = heif_image_handle_get_luma_bits_per_pixel(handle);
     *                heif_image_handle_get_luma_bits_per_pixel
    if (bit_depth < 0) {
        heif_decoding_options_free(decode_options);
        heif_image_handle_release(handle);
        std::cerr << "Input image has undefined bit-depth\n";
        return 1;
    }

    struct heif_image* image;
    error = heif_decode_image(handle,
                            &image,
                            encoder->colorspace(has_alpha),
                            encoder->chroma(has_alpha, bit_depth),
                            decode_options);
    heif_decoding_options_free(decode_options);*/

    heif_image* image;
    error = heif_decode_image(handle,
                              &image,
                              heif_colorspace_RGB,
                              heif_chroma_interleaved_RGBA,
                              nullptr);
//    heif_image* image;
//    error = heif_decode_image(handle, &image, heif_colorspace_RGB, heif_chroma_interleaved_RGBA, nullptr);
    if (error.code != heif_error_Ok) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "decode_image failed code:%d msg:%s path:%s",error.code, error.message, srcPath);
        heif_image_handle_release(handle);
        heif_context_free(ctx);
        return NULL;
    }

    int width = heif_image_handle_get_width(handle);
    int height = heif_image_handle_get_height(handle);

    int stride = 0;
    const uint8_t* data = heif_image_get_plane_readonly(image, heif_channel_interleaved, &stride);
    __android_log_print(ANDROID_LOG_DEBUG, TAG, "decode image %dx%d, stride = %d", width, height, stride);

    jbyteArray array = env->NewByteArray(stride*height);
    env->SetByteArrayRegion(array, 0, stride*height, reinterpret_cast<const jbyte *>(data));

    env->CallVoidMethod(outSize, env->GetMethodID(env->GetObjectClass(outSize), "setWidth", "(I)V"), width);
    env->CallVoidMethod(outSize, env->GetMethodID(env->GetObjectClass(outSize), "setHeight", "(I)V"), height);

    //资源回收
    heif_image_release(image);
    heif_image_handle_release(handle);
    heif_context_free(ctx);

    env->ReleaseStringUTFChars(srcPath_, srcPath);

    return array;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_strod_heif_HeifNative_heif2jpg(JNIEnv *env, jclass type, jstring srcPath_, jstring outPath_){

    std::unique_ptr<Encoder> encoder;
    static const int kDefaultJpegQuality = 100;
    encoder.reset(new JpegEncoder(kDefaultJpegQuality));

    const char *srcPath = env->GetStringUTFChars(srcPath_, 0);

    // --- read the HEIF file
    heif_context* ctx = heif_context_alloc();
    if (!ctx) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG,"Could not create context object");
        return false;
    }
    heif_error error;
    error = heif_context_read_from_file(ctx, srcPath, nullptr);
    if (error.code != heif_error_Ok) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "read_from_file failed code:%d msg:%s path:%s",error.code, error.message, srcPath);
        heif_context_free(ctx);
        return false;
    }

    int num_images = heif_context_get_number_of_top_level_images(ctx);
    if (num_images == 0) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "File doesn't contain any images:%s",error.code, error.message, srcPath);
        return false;
    }

    heif_image_handle* handle;
    error = heif_context_get_primary_image_handle(ctx, &handle);
    if (error.code != heif_error_Ok) {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "get_primary_image_handle failed code:%d msg:%s path:%s",error.code, error.message, srcPath);
        heif_context_free(ctx);
        return false;
    }

    int has_alpha = heif_image_handle_has_alpha_channel(handle);
    struct heif_decoding_options* decode_options = heif_decoding_options_alloc();
    encoder->UpdateDecodingOptions(handle, decode_options);

    int bit_depth = heif_image_handle_get_luma_bits_per_pixel(handle);
    if (bit_depth < 0) {
        heif_decoding_options_free(decode_options);
        heif_image_handle_release(handle);
//        std::cerr << "Input image has undefined bit-depth\n";
        return false;
    }

    struct heif_image* image;
    error = heif_decode_image(handle,
                            &image,
                            encoder->colorspace(has_alpha),
                            encoder->chroma(has_alpha, bit_depth),
                            decode_options);
    heif_decoding_options_free(decode_options);
    if (error.code) {
        heif_image_handle_release(handle);
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "decode_image failed code:%d msg:%s path:%s",error.code, error.message, srcPath);
        return false;
    }
    if (image) {
        const char *outPath = env->GetStringUTFChars(outPath_, 0);
        bool written = encoder->Encode(handle, image, outPath);
        if (!written) {
            fprintf(stderr, "could not write image\n");
            __android_log_print(ANDROID_LOG_DEBUG, TAG, "could not write image path:%s", outPath);
            return false;
        }
        heif_image_release(image);
        return true;
    }
    return false;
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_strod_heif_HeifNative_encodeYUV(JNIEnv *env, jclass type, jbyteArray bytes_,
                                                  jint width, jint height, jstring outputPath_) {
    jbyte *bytes = env->GetByteArrayElements(bytes_, NULL);
    const char *outputPath = env->GetStringUTFChars(outputPath_, 0);

    heif_image* image;
    heif_image_create(width, height, heif_colorspace_YCbCr, heif_chroma_420, &image);
    heif_image_add_plane(image, heif_channel_Y, width, height, 8);
    heif_image_add_plane(image, heif_channel_Cb, width/2, height/2, 8);
    heif_image_add_plane(image, heif_channel_Cr, width/2, height/2, 8);

    int sy, su, sv;
    uint8_t* py = heif_image_get_plane(image, heif_channel_Y, &sy);
    uint8_t* pu = heif_image_get_plane(image, heif_channel_Cb, &su);
    uint8_t* pv = heif_image_get_plane(image, heif_channel_Cr, &sv);

    std::memcpy(py, bytes, static_cast<size_t>(width * height));
    std::memcpy(pu, bytes+(width*height), static_cast<size_t>(width * height / 4));
    std::memcpy(pv, bytes+(width*height+width*height/4), static_cast<size_t>(width * height / 4));

    heif_context* ctx = heif_context_alloc();
    heif_encoder* encoder;
    heif_context_get_encoder_for_format(ctx, heif_compression_HEVC, &encoder);

    heif_encoding_options* options = heif_encoding_options_alloc();
    options->save_alpha_channel = 0;

    heif_image_handle* handle;
    heif_context_encode_image(ctx, image, encoder, options, &handle);
    heif_encoder_release(encoder);

    heif_error error;
    error = heif_context_write_to_file(ctx, outputPath);
    if (error.code != heif_error_Ok) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "YUV write to file error %s", error.message);
    } else {
        __android_log_print(ANDROID_LOG_DEBUG, TAG, "YUV write to file success");
    }

    heif_image_handle_release(handle);
    heif_image_release(image);
    heif_context_free(ctx);

    env->ReleaseByteArrayElements(bytes_, bytes, 0);
    env->ReleaseStringUTFChars(outputPath_, outputPath);

    return error.code;
}