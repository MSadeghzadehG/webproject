

package com.badlogic.gdx.backends.gwt.soundmanager2;

import com.badlogic.gdx.Gdx;
import com.badlogic.gdx.audio.Music.OnCompletionListener;
import com.badlogic.gdx.backends.gwt.GwtMusic;
import com.badlogic.gdx.backends.gwt.soundmanager2.SoundManager.SoundManagerCallback;
import com.google.gwt.core.client.JavaScriptObject;

public class SMSound {
	
	public interface SMSoundCallback {
		public void onfinish ();
	}
	
	
	public static final int STOPPED = 0;
	public static final int PLAYING = 1;
	
	private JavaScriptObject jsSound;
	
	protected SMSound (JavaScriptObject jsSound) {
		this.jsSound = jsSound;
	}

	
	public native final void destruct () ;

	
	public native final int getPosition () ;
	
	
	public native final void setPosition (int position) ;
	
	
	public native final void pause () ;
	
	
	public native final void play (SMSoundOptions options) ;
	
	
	public native final void play () ;

	
	public native final void resume () ;

	
	public native final void stop () ;

	
	public native final void setVolume (int volume) ;

	
	public native final int getVolume () ;

	
	public native final void setPan (int pan) ;
	
	
	public native final int getPan () ;
	
	
	public native final int getPlayState () ;
	
	
	public native final boolean getPaused () ;
	
	
	public native final int getLoops () ;
}