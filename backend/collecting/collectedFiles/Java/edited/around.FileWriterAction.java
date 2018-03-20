
package com.iluwatar.execute.around;

import java.io.FileWriter;
import java.io.IOException;


public interface FileWriterAction {

  void writeFile(FileWriter writer) throws IOException;

}
