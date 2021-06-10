package com.strod.heif;

/**
 * Created by laiying on 2021/6/2.
 */
public class HeifSize {

    private int width;
    private int height;

    public void setWidth(int width) {
        this.width = width;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    public int getWidth() {
        return width;
    }

    public int getHeight() {
        return height;
    }

    @Override
    public String toString() {
        return "HeifSize{" +
                "width=" + width +
                ", height=" + height +
                '}';
    }
}
