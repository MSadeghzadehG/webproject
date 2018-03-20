
package com.badlogic.gdx.backends.iosrobovm.custom;


import java.io.*;
import java.nio.*;
import java.util.*;
import org.robovm.objc.*;
import org.robovm.objc.annotation.*;
import org.robovm.objc.block.*;
import org.robovm.rt.*;
import org.robovm.rt.bro.*;
import org.robovm.rt.bro.annotation.*;
import org.robovm.rt.bro.ptr.*;
import org.robovm.apple.foundation.*;
import org.robovm.apple.coreanimation.*;
import org.robovm.apple.coregraphics.*;
import org.robovm.apple.coredata.*;
import org.robovm.apple.coreimage.*;
import org.robovm.apple.coretext.*;
import org.robovm.apple.corelocation.*;




@Deprecated

@Library("UIKit") @NativeClass
public class UIAccelerometer 
    extends NSObject 
    public static class UIAccelerometerPtr extends Ptr<UIAccelerometer, UIAccelerometerPtr> {}
    static { ObjCRuntime.bind(UIAccelerometer.class); }
    
    public UIAccelerometer() {}
    protected UIAccelerometer(SkipInit skipInit) { super(skipInit); }
    
    
    @Property(selector = "updateInterval")
    public native double getUpdateInterval();
    @Property(selector = "setUpdateInterval:")
    public native void setUpdateInterval(double v);
    @Property(selector = "delegate")
    public native UIAccelerometerDelegate getDelegate();
    @Property(selector = "setDelegate:", strongRef = true)
    public native void setDelegate(UIAccelerometerDelegate v);
    
    
    @Method(selector = "sharedAccelerometer")
    public static native UIAccelerometer getSharedAccelerometer();
    
}