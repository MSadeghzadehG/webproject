

package com.iluwatar.event.queue;

import java.io.File;
import java.io.IOException;

import javax.sound.sampled.AudioInputStream;
import javax.sound.sampled.AudioSystem;
import javax.sound.sampled.Clip;
import javax.sound.sampled.LineUnavailableException;
import javax.sound.sampled.UnsupportedAudioFileException;


public class Audio {

  private static final int MAX_PENDING = 16;

  private static int headIndex;

  private static int tailIndex;

  private static Thread updateThread = null;

  private static PlayMessage[] pendingAudio = new PlayMessage[MAX_PENDING];

  
  public static synchronized void stopService() {
    if (updateThread != null) {
      updateThread.interrupt();
    }
  }
  
  
  public static synchronized boolean isServiceRunning() {
    if (updateThread != null && updateThread.isAlive() ) {
      return true;
    } else {
      return false;
    }
  }

  
  public static void init() {
    if (updateThread == null) {
      updateThread = new Thread(new Runnable() {
        public void run() {
          while (!Thread.currentThread().isInterrupted()) {
            Audio.update();
          }
        }
      });
    }
    startThread();
  }
  
  
  public static synchronized void startThread() {
    if (!updateThread.isAlive()) {
      updateThread.start();
      headIndex = 0;
      tailIndex = 0;
    }
  }

  
  public static void playSound(AudioInputStream stream, float volume) {
    init();
        for (int i = headIndex; i != tailIndex; i = (i + 1) % MAX_PENDING) {
      if (getPendingAudio()[i].getStream() == stream) {
                getPendingAudio()[i].setVolume(Math.max(volume, getPendingAudio()[i].getVolume()));

                return;
      }
    }
    getPendingAudio()[tailIndex] = new PlayMessage(stream, volume);
    tailIndex = (tailIndex + 1) % MAX_PENDING;
  }
  
  
  public static void update() {
        if (headIndex == tailIndex) {
      return;
    }
    Clip clip = null;
    try {
      AudioInputStream audioStream = getPendingAudio()[headIndex].getStream();
      headIndex++;
      clip = AudioSystem.getClip();
      clip.open(audioStream);
      clip.start();
    } catch (LineUnavailableException e) {
      System.err.println("Error occoured while loading the audio: The line is unavailable");
      e.printStackTrace();
    } catch (IOException e) {
      System.err.println("Input/Output error while loading the audio");
      e.printStackTrace();
    } catch (IllegalArgumentException e) {
      System.err.println("The system doesn't support the sound: " + e.getMessage());
    }
  }

  
  public static AudioInputStream getAudioStream(String filePath) 
      throws UnsupportedAudioFileException, IOException {
    return AudioSystem.getAudioInputStream(new File(filePath).getAbsoluteFile());
  }

  
  public static PlayMessage[] getPendingAudio() {
    return pendingAudio;
  }

}
