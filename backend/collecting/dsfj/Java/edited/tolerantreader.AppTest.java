
package com.iluwatar.tolerantreader;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.io.File;
import java.io.IOException;


public class AppTest {

  @Test
  public void test() throws ClassNotFoundException, IOException {
    String[] args = {};
    App.main(args);
  }

  @BeforeEach
  @AfterEach
  public void cleanup() {
    File file1 = new File("fish1.out");
    file1.delete();
    File file2 = new File("fish2.out");
    file2.delete();
  }
}
