package com.strod.heif;

import android.content.Context;
import android.graphics.Bitmap;
import android.text.TextUtils;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/**
 * Created by laiying on 2021/6/2.
 */
public class HEIFUtils {
    private HEIFUtils() {
        //no instance
    }

    private static final String TAG = "HEIFUtils";

    /**
     * Encode YUV data to HEIF
     * @param yuvBytes yuv bytes
     * @param width width
     * @param height height
     * @param outputPath output path
     * @return true if success
     */
    public static boolean encodeYUVToHEIF(final byte[] yuvBytes, final int width, final int height, final String outputPath) {
        return HeifNative.encodeYUV(yuvBytes, width, height, outputPath) == 0;
    }

    /**
     * Encode bitmap to HEIF
     * @param bitmap source bitmap
     * @param outputPath output heif path
     * @return true if success
     */
    public static boolean encodeBitmapToHEIF(final Bitmap bitmap, final String outputPath) {
        if (bitmap != null && !TextUtils.isEmpty(outputPath)) {
            ByteBuffer buffer = ByteBuffer.allocate(bitmap.getByteCount());
            bitmap.copyPixelsToBuffer(buffer);
            byte[] bytes = buffer.array();
            return HeifNative.encodeBitmap(bytes, bitmap.getWidth(), bitmap.getHeight(), outputPath) == 0;
        }
        return false;
    }

    /**
     * Decode a heif to bitmap
     * @param heifFilePath source file path
     * @return a decoded bitmap or null if the provided file path is invalid
     */
    public static Bitmap decodeHEIFToBitmap(String heifFilePath) {
        if (TextUtils.isEmpty(heifFilePath) || !new File(heifFilePath).exists()) {
            Log.e(TAG, "decodeHEIFToBitmap: file " + heifFilePath + " not exists");
            return null;
        }

        HeifSize heifSize = new HeifSize();
        byte[] buffer = HeifNative.decodeHeif2RGBA(heifSize, heifFilePath);
        Bitmap bitmap = Bitmap.createBitmap(heifSize.getWidth(), heifSize.getHeight(), Bitmap.Config.ARGB_8888);
        bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(buffer));
        return bitmap;
    }

    /**
     * Convert a heif to bitmap
     * @param heifFilePath source file path
     * @param jpgFilePath source file path
     * @return true if success
     */
    public static boolean convertHEIFToJpg(String heifFilePath, String jpgFilePath) {
        long start = System.currentTimeMillis();
        if (TextUtils.isEmpty(heifFilePath) || !new File(heifFilePath).exists()) {
            Log.e(TAG, "convertHEIFToJpg: file " + heifFilePath + " not exists");
            return false;
        }

        HeifSize heifSize = new HeifSize();
        byte[] buffer = HeifNative.decodeHeif2RGBA(heifSize, heifFilePath);
        if (buffer == null){
            return false;
        }
        Bitmap bitmap = null;
        try {
            bitmap = Bitmap.createBitmap(heifSize.getWidth(), heifSize.getHeight(), Bitmap.Config.ARGB_8888);
            bitmap.copyPixelsFromBuffer(ByteBuffer.wrap(buffer));
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }

        File file = new File(jpgFilePath);
        FileOutputStream out = null;
        try {
            out = new FileOutputStream(file);
            if (bitmap.compress(Bitmap.CompressFormat.JPEG, 100, out)) {
                out.flush();
                long end = System.currentTimeMillis();
                Log.e(TAG, "convertHEIFToJpg : file " + heifFilePath + " used "+(end - start) +" ms");
                return true;
            }
        } catch (IOException e) {
            e.printStackTrace();
        }finally {
            if (out != null){
                try {
                    out.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return false;
    }
    /**
     * Convert HEIF to JPEG
     * @param heifFilePath
     * @param jpegFilePath
     * @return
     */
    public static boolean heif2jpg(String heifFilePath, String jpegFilePath){
        long start = System.currentTimeMillis();
        if (TextUtils.isEmpty(heifFilePath) || !new File(heifFilePath).exists()) {
            Log.e(TAG, "heif2jpg: file " + heifFilePath + " not exists");
            return false;
        }
        boolean isSuccess = HeifNative.heif2jpg(heifFilePath, jpegFilePath);
        long end = System.currentTimeMillis();
        Log.e(TAG, "heif2jpg : file " + heifFilePath + " used "+(end - start) +" ms");
        return isSuccess;
    }


    /**
     * Get byte content of an asset name
     * @param context the application context
     * @param assetName asset name
     * @return byte array represent the content of asset
     */
    public static byte[] bytesOfAssetName(Context context, String assetName) {
        if (context != null && !TextUtils.isEmpty(assetName)) {
            InputStream assetInputStream = null;
            try {
                assetInputStream = context.getAssets().open(assetName);
                byte[] bytes = new byte[assetInputStream.available()];
                if (assetInputStream.read(bytes) > 0) {
                    return bytes;
                }
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                if (assetInputStream != null) {
                    try {
                        assetInputStream.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        return null;
    }

    /**
     * Retrieve an accessible path of an asset name
     * @param context the application context
     * @param assetName asset name
     * @return an absolute path inside app private storage
     */
    public static String accessiblePathOfAssetName(Context context, String assetName) {
        String fileName = String.valueOf(System.currentTimeMillis());
        try {
            MessageDigest digest = MessageDigest.getInstance("MD5");
            fileName = new String(digest.digest(fileName.getBytes()));
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }

        if (context != null && !TextUtils.isEmpty(assetName)) {
            final String absoluteFilePath = context.getFilesDir().getAbsolutePath().concat(File.separator).concat(fileName);

            InputStream assetInputStream = null;
            FileOutputStream fileOutputStream = null;
            try {
                assetInputStream = context.getAssets().open(assetName);
                fileOutputStream = new FileOutputStream(absoluteFilePath);

                byte[] bytes = new byte[assetInputStream.available()];
                if (assetInputStream.read(bytes) > 0) {
                    fileOutputStream.write(bytes);
                }
                fileOutputStream.flush();
                return absoluteFilePath;
            } catch (IOException e) {
                e.printStackTrace();
            } finally {
                if (assetInputStream != null) {
                    try {
                        assetInputStream.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }

                if (fileOutputStream != null) {
                    try {
                        fileOutputStream.close();
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            }
        }

        return null;
    }
}
