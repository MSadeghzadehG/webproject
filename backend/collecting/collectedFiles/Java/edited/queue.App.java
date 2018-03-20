

package com.iluwatar.event.queue;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

import javax.sound.sampled.UnsupportedAudioFileException;


public class App {
  
  public static void main(String[] args) throws UnsupportedAudioFileException, IOException {
    Audio.playSound(Audio.getAudioStream("./etc/Bass-Drum-1.wav"), -10.0f);
    Audio.playSound(Audio.getAudioStream("./etc/Closed-Hi-Hat-1.wav"), -8.0f);
    
    System.out.println("Press Enter key to stop the program...");
    BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
    br.read();
    Audio.stopService();
  }
}
