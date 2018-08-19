
package com.iluwatar.reactor.framework;

import java.nio.channels.SelectionKey;


public class SameThreadDispatcher implements Dispatcher {

  
  @Override
  public void onChannelReadEvent(AbstractNioChannel channel, Object readObject, SelectionKey key) {
    
    channel.getHandler().handleChannelRead(channel, readObject, key);
  }

  
  @Override
  public void stop() {
      }
}
