
package com.iluwatar.execute.around;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import java.io.File;
import java.io.IOException;


public class AppTest {

  @Test
  public void test() throws IOException {
    String[] args = {};
    App.main(args);
  }

  @BeforeEach
  @AfterEach
  public void cleanup() {
    File file = new File("testfile.txt");
    file.delete();
  }
}
