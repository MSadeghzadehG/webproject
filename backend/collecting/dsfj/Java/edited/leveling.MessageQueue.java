
package com.iluwatar.queue.load.leveling;

import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class MessageQueue {
  
  private static final Logger LOGGER = LoggerFactory.getLogger(App.class);
  
  private final BlockingQueue<Message> blkQueue;
  
    public MessageQueue() {
    this.blkQueue = new ArrayBlockingQueue<Message>(1024);
  }
  
  
  public void submitMsg(Message msg) {
    try {
      if (null != msg) {
        blkQueue.add(msg);
      }
    } catch (Exception e) {
      LOGGER.error(e.getMessage());
    }
  }
  
  
  public Message retrieveMsg() {
    Message retrievedMsg = null;
    try {
      retrievedMsg = blkQueue.poll();
    } catch (Exception e) {
      LOGGER.error(e.getMessage());
    }
    
    return retrievedMsg;
  }
}