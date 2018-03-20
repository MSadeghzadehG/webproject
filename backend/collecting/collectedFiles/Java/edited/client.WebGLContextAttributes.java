

package com.google.gwt.webgl.client;

import com.google.gwt.core.client.JavaScriptObject;


public class WebGLContextAttributes extends JavaScriptObject {

	public static native WebGLContextAttributes create () ;

	protected WebGLContextAttributes () {
	}

	
	public final native void setAlpha (boolean alpha) ;

	public final native void clearAlpha () ;

	
	public final native void setDepth (boolean depth) ;

	public final native void clearDepth () ;

	
	public final native void setStencil (boolean stencil) ;

	public final native void clearStencil () ;

	
	public final native void setAntialias (boolean antialias) ;

	public final native void clearAntialias () ;

	
	public final native void setPremultipliedAlpha (boolean premultipliedAlpha) ;

	public final native void clearPremultipliedAlpha () ;
	
	public final native void setPreserveDrawingBuffer (boolean preserveDrawingBuffer) ;
}
