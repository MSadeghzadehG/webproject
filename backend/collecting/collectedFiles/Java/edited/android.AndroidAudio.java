

package com.badlogic.gdx.backends.android;

import android.app.Activity;
import android.content.Context;
import android.content.res.AssetFileDescriptor;
import android.media.AudioAttributes;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.SoundPool;
import android.os.Build;

import com.badlogic.gdx.Audio;
import com.badlogic.gdx.Files.FileType;
import com.badlogic.gdx.audio.AudioDevice;
import com.badlogic.gdx.audio.AudioRecorder;
import com.badlogic.gdx.audio.Music;
import com.badlogic.gdx.audio.Sound;
import com.badlogic.gdx.files.FileHandle;
import com.badlogic.gdx.utils.GdxRuntimeException;

import java.io.FileDescriptor;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


public final class AndroidAudio implements Audio {
	private final SoundPool soundPool;
	private final AudioManager manager;
	protected final List<AndroidMusic> musics = new ArrayList<AndroidMusic>();

	public AndroidAudio (Context context, AndroidApplicationConfiguration config) {
		if (!config.disableAudio) {
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
				AudioAttributes audioAttrib = new AudioAttributes.Builder()
						.setUsage(AudioAttributes.USAGE_GAME)
						.setContentType(AudioAttributes.CONTENT_TYPE_SONIFICATION)
						.build();
				soundPool = new SoundPool.Builder().setAudioAttributes(audioAttrib).setMaxStreams(config.maxSimultaneousSounds).build();
			}else {
				soundPool = new SoundPool(config.maxSimultaneousSounds, AudioManager.STREAM_MUSIC, 0);			}
			manager = (AudioManager)context.getSystemService(Context.AUDIO_SERVICE);
			if (context instanceof Activity) {
				((Activity)context).setVolumeControlStream(AudioManager.STREAM_MUSIC);
			}
		} else {
			soundPool = null;
			manager = null;
		}
	}

	protected void pause () {
		if (soundPool == null) {
			return;
		}
		synchronized (musics) {
			for (AndroidMusic music : musics) {
				if (music.isPlaying()) {
					music.pause();
					music.wasPlaying = true;					
				} else
					music.wasPlaying = false;
			}
		}
		this.soundPool.autoPause();
	}

	protected void resume () {
		if (soundPool == null) {
			return;
		}
		synchronized (musics) {
			for (int i = 0; i < musics.size(); i++) {
				if (musics.get(i).wasPlaying) musics.get(i).play();
			}
		}
		this.soundPool.autoResume();
	}

	
	@Override
	public AudioDevice newAudioDevice (int samplingRate, boolean isMono) {
		if (soundPool == null) {
			throw new GdxRuntimeException("Android audio is not enabled by the application config.");
		}
		return new AndroidAudioDevice(samplingRate, isMono);
	}

	
	@Override
	public Music newMusic (FileHandle file) {
		if (soundPool == null) {
			throw new GdxRuntimeException("Android audio is not enabled by the application config.");
		}
		AndroidFileHandle aHandle = (AndroidFileHandle)file;

		MediaPlayer mediaPlayer = new MediaPlayer();

		if (aHandle.type() == FileType.Internal) {
			try {
				AssetFileDescriptor descriptor = aHandle.getAssetFileDescriptor();
				mediaPlayer.setDataSource(descriptor.getFileDescriptor(), descriptor.getStartOffset(), descriptor.getLength());
				descriptor.close();
				mediaPlayer.prepare();
				AndroidMusic music = new AndroidMusic(this, mediaPlayer);
				synchronized (musics) {
					musics.add(music);
				}
				return music;
			} catch (Exception ex) {
				throw new GdxRuntimeException("Error loading audio file: " + file
					+ "\nNote: Internal audio files must be placed in the assets directory.", ex);
			}
		} else {
			try {
				mediaPlayer.setDataSource(aHandle.file().getPath());
				mediaPlayer.prepare();
				AndroidMusic music = new AndroidMusic(this, mediaPlayer);
				synchronized (musics) {
					musics.add(music);
				}
				return music;
			} catch (Exception ex) {
				throw new GdxRuntimeException("Error loading audio file: " + file, ex);
			}
		}

	}

	
	public Music newMusic (FileDescriptor fd) {
		if (soundPool == null) {
			throw new GdxRuntimeException("Android audio is not enabled by the application config.");
		}
		
		MediaPlayer mediaPlayer = new MediaPlayer();

		try {
			mediaPlayer.setDataSource(fd);
			mediaPlayer.prepare();

			AndroidMusic music = new AndroidMusic(this, mediaPlayer);
			synchronized (musics) {
				musics.add(music);
			}
			return music;
		} catch (Exception ex) {
			throw new GdxRuntimeException("Error loading audio from FileDescriptor", ex);
		}
	}
	
	
	@Override
	public Sound newSound (FileHandle file) {
		if (soundPool == null) {
			throw new GdxRuntimeException("Android audio is not enabled by the application config.");
		}
		AndroidFileHandle aHandle = (AndroidFileHandle)file;
		if (aHandle.type() == FileType.Internal) {
			try {
				AssetFileDescriptor descriptor = aHandle.getAssetFileDescriptor();
				AndroidSound sound = new AndroidSound(soundPool, manager, soundPool.load(descriptor, 1));
				descriptor.close();
				return sound;
			} catch (IOException ex) {
				throw new GdxRuntimeException("Error loading audio file: " + file
					+ "\nNote: Internal audio files must be placed in the assets directory.", ex);
			}
		} else {
			try {
				return new AndroidSound(soundPool, manager, soundPool.load(aHandle.file().getPath(), 1));
			} catch (Exception ex) {
				throw new GdxRuntimeException("Error loading audio file: " + file, ex);
			}
		}
	}

	
	@Override
	public AudioRecorder newAudioRecorder (int samplingRate, boolean isMono) {
		if (soundPool == null) {
			throw new GdxRuntimeException("Android audio is not enabled by the application config.");
		}
		return new AndroidAudioRecorder(samplingRate, isMono);
	}

	
	public void dispose () {
		if (soundPool == null) {
			return;
		}
		synchronized (musics) {
						ArrayList<AndroidMusic> musicsCopy = new ArrayList<AndroidMusic>(musics);
			for (AndroidMusic music : musicsCopy) {
				music.dispose();
			}
		}
		soundPool.release();
	}
}
