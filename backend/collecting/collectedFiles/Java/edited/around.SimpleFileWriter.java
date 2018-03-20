
package com.iluwatar.execute.around;

import java.io.FileWriter;
import java.io.IOException;


public class SimpleFileWriter {

  
  public SimpleFileWriter(String filename, FileWriterAction action) throws IOException {
    FileWriter writer = new FileWriter(filename);
    try {
      action.writeFile(writer);
    } finally {
      writer.close();
    }
  }
}
