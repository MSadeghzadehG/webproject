

package com.iluwatar.event.queue;

import org.junit.jupiter.api.Test;

import javax.sound.sampled.UnsupportedAudioFileException;
import java.io.IOException;

import static org.junit.jupiter.api.Assertions.assertFalse;
import static org.junit.jupiter.api.Assertions.assertTrue;



public class AudioTest {

  
  @Test
  public void testPlaySound() throws UnsupportedAudioFileException, IOException, InterruptedException {
    Audio.playSound(Audio.getAudioStream("./etc/Bass-Drum-1.wav"), -10.0f);
        assertTrue(Audio.isServiceRunning());
        Thread.sleep(5000);
        assertFalse(!Audio.isServiceRunning());
  }

  
  @Test
  public void testQueue() throws UnsupportedAudioFileException, IOException, InterruptedException {
    Audio.playSound(Audio.getAudioStream("./etc/Bass-Drum-1.aif"), -10.0f);
    Audio.playSound(Audio.getAudioStream("./etc/Bass-Drum-1.aif"), -10.0f);
    Audio.playSound(Audio.getAudioStream("./etc/Bass-Drum-1.aif"), -10.0f);
    assertTrue(Audio.getPendingAudio().length > 0);
        assertTrue(Audio.isServiceRunning());
        Thread.sleep(10000);
        assertFalse(!Audio.isServiceRunning());
  }

}
