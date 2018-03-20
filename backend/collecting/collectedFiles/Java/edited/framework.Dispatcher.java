
package com.iluwatar.reactor.framework;

import java.nio.channels.SelectionKey;


public interface Dispatcher {
  
  void onChannelReadEvent(AbstractNioChannel channel, Object readObject, SelectionKey key);

  
  void stop() throws InterruptedException;
}
