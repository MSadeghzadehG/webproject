

package com.iluwatar.mute;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.sql.SQLException;


public class App {

  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);

  
  public static void main(String[] args) throws Exception {

    useOfLoggedMute();

    useOfMute();
  }

  
  private static void useOfMute() {
    ByteArrayOutputStream out = new ByteArrayOutputStream();
    Mute.mute(() -> out.write("Hello".getBytes()));
  }

  private static void useOfLoggedMute() throws SQLException {
    Resource resource = null;
    try {
      resource = acquireResource();
      utilizeResource(resource);
    } finally {
      closeResource(resource);
    }
  }

  
  private static void closeResource(Resource resource) {
    Mute.loggedMute(() -> resource.close());
  }

  private static void utilizeResource(Resource resource) throws SQLException {
    LOGGER.info("Utilizing acquired resource: {}", resource);
  }

  private static Resource acquireResource() throws SQLException {
    return new Resource() {
      
      @Override
      public void close() throws IOException {
        throw new IOException("Error in closing resource: " + this);
      }
    };
  }
}
