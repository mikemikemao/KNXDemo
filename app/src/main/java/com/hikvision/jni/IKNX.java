package com.hikvision.jni;

public class IKNX {
    static {
        System.loadLibrary("IKNX");
    }
    /**
     * description startPreview
     * param surface
     * @return
     */
    public native void native_openLight();
}
