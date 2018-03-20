

package com.badlogic.gdx;

import com.badlogic.gdx.utils.Clipboard;


public interface Application {
	
	public enum ApplicationType {
		Android, Desktop, HeadlessDesktop, Applet, WebGL, iOS
	}

	public static final int LOG_NONE = 0;
	public static final int LOG_DEBUG = 3;
	public static final int LOG_INFO = 2;
	public static final int LOG_ERROR = 1;

	
	public ApplicationListener getApplicationListener ();

	
	public Graphics getGraphics ();

	
	public Audio getAudio ();

	
	public Input getInput ();

	
	public Files getFiles ();

	
	public Net getNet ();

	
	public void log (String tag, String message);

	
	public void log (String tag, String message, Throwable exception);

	
	public void error (String tag, String message);

	
	public void error (String tag, String message, Throwable exception);

	
	public void debug (String tag, String message);

	
	public void debug (String tag, String message, Throwable exception);

	
	public void setLogLevel (int logLevel);

	
	public int getLogLevel ();

	
	public void setApplicationLogger (ApplicationLogger applicationLogger);

	
	public ApplicationLogger getApplicationLogger ();

	
	public ApplicationType getType ();

	
	public int getVersion ();

	
	public long getJavaHeap ();

	
	public long getNativeHeap ();

	
	public Preferences getPreferences (String name);

	public Clipboard getClipboard ();

	
	public void postRunnable (Runnable runnable);

	
	public void exit ();

	
	public void addLifecycleListener (LifecycleListener listener);

	
	public void removeLifecycleListener (LifecycleListener listener);
}
