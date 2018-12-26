
package com.iluwatar.pageobject;

import java.awt.Desktop;
import java.io.File;
import java.io.IOException;


public final class App {

  private App() {
  }

  
  public static void main(String[] args) {

    try {
      File applicationFile = new File(App.class.getClassLoader().getResource("sample-ui/login.html").getPath());

            if (Desktop.isDesktopSupported()) {
        Desktop.getDesktop().open(applicationFile);

      } else {
                Runtime.getRuntime().exec("cmd.exe start " + applicationFile);
      }

    } catch (IOException ex) {
      ex.printStackTrace();
    }

  }
}
