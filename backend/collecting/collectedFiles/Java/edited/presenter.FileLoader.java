
package com.iluwatar.model.view.presenter;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.Serializable;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class FileLoader implements Serializable {

  
  private static final long serialVersionUID = -4745803872902019069L;
  
  private static final Logger LOGGER = LoggerFactory.getLogger(FileLoader.class);

  
  private boolean loaded;

  
  private String fileName;

  
  public String loadData() {
    String dataFileName = this.fileName;
    try (BufferedReader br = new BufferedReader(new FileReader(new File(dataFileName)))) {
      StringBuilder sb = new StringBuilder();
      String line;

      while ((line = br.readLine()) != null) {
        sb.append(line).append('\n');
      }

      this.loaded = true;

      return sb.toString();
    } catch (Exception e) {
      LOGGER.error("File {} does not exist", dataFileName);
    }

    return null;
  }

  
  public void setFileName(String fileName) {
    this.fileName = fileName;
  }

  
  public String getFileName() {
    return this.fileName;
  }

  
  public boolean fileExists() {
    return new File(this.fileName).exists();
  }

  
  public boolean isLoaded() {
    return this.loaded;
  }
}
