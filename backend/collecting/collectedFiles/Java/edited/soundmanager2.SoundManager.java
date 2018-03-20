

package com.badlogic.gdx.backends.gwt.soundmanager2;

import com.google.gwt.core.client.JavaScriptObject;

public class SoundManager {
	
	public interface SoundManagerCallback {
		public void onready ();
		public void ontimeout (String status, String errorType);
	}

	public static final native SoundManager getInstance () ;

	public static native String getVersion () ;

	public static native String getUrl () ;

	public static native void setUrl (String url) ;

	public static native void setDebugMode (boolean debug) ;

	public static native boolean getDebugMode () ;

	public static native void setFlashVersion (int version) ;

	public static native int getFlashVersion () ;

	
	public static native SMSound createSound (String url) ;

	public static native void reboot () ;

	public static native boolean ok () ;

	public static native void init (String moduleBaseURL, int flashVersion, boolean preferFlash, SoundManagerCallback callback) ;

}