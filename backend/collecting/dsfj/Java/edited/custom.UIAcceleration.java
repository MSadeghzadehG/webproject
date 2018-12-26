
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
public class UIAcceleration 
    extends NSObject 
    public static class UIAccelerationPtr extends Ptr<UIAcceleration, UIAccelerationPtr> {}
    static { ObjCRuntime.bind(UIAcceleration.class); }
    
    public UIAcceleration() {}
    protected UIAcceleration(SkipInit skipInit) { super(skipInit); }
    
    
    @Property(selector = "timestamp")
    public native double getTimestamp();
    @Property(selector = "x")
    public native double getX();
    @Property(selector = "y")
    public native double getY();
    @Property(selector = "z")
    public native double getZ();
    
    
    
    
}