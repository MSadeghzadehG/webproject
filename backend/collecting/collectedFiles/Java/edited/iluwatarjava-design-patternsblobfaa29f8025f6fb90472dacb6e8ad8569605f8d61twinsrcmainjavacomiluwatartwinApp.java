
package com.iluwatar.twin;



public class App {

  
  public static void main(String[] args) throws Exception {

    BallItem ballItem = new BallItem();
    BallThread ballThread = new BallThread();

    ballItem.setTwin(ballThread);
    ballThread.setTwin(ballItem);

    ballThread.start();

    waiting();

    ballItem.click();

    waiting();

    ballItem.click();

    waiting();

        ballThread.stopMe();
  }

  private static void waiting() throws Exception {
    Thread.sleep(750);
  }
}
