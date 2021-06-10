package com.strod.heif;

/**
 * Created by laiying on 2021/6/2.
 */
public class HeifNative {

    static {
        System.loadLibrary("jpeg");
        System.loadLibrary("heif");
        System.loadLibrary("heif_jni");
    }

    /**
     * Encode rgba bytes to HEIF
     * @param bytes rgba bytes
     * @param width width
     * @param height height
     * @param outputPath output path
     * @return 0 if success
     */
    public static native int encodeBitmap(byte[] bytes, int width, int height, String outputPath);

    /**
     * Encode YUV bytes to HEIF
     * @param bytes yuv bytes
     * @param width desired width
     * @param height desired height
     * @param outputPath output path
     * @return 0 if success
     */
    public static native int encodeYUV(byte[] bytes, int width, int height, String outputPath);

    /**
     * Decode HEIF to rgba bytes
     * @param outSize output size
     * @param srcPath source path to decode
     * @return rgba byte, convenient to create a {@link android.graphics.Bitmap}
     */
    public static native byte[] decodeHeif2RGBA(HeifSize outSize, String srcPath);


    /**
     * Convert HEIF to JPEG
     * @param srcPath
     * @param outPath
     * @return
     */
    public static native boolean heif2jpg(String srcPath, String outPath);
}
