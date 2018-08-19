
package com.iluwatar.reactor.app;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import com.iluwatar.reactor.framework.AbstractNioChannel;
import com.iluwatar.reactor.framework.ChannelHandler;
import com.iluwatar.reactor.framework.Dispatcher;
import com.iluwatar.reactor.framework.NioDatagramChannel;
import com.iluwatar.reactor.framework.NioReactor;
import com.iluwatar.reactor.framework.NioServerSocketChannel;
import com.iluwatar.reactor.framework.ThreadPoolDispatcher;


public class App {

  private NioReactor reactor;
  private List<AbstractNioChannel> channels = new ArrayList<>();
  private Dispatcher dispatcher;

  
  public App(Dispatcher dispatcher) {
    this.dispatcher = dispatcher;
  }

  
  public static void main(String[] args) throws IOException {
    new App(new ThreadPoolDispatcher(2)).start();
  }

  
  public void start() throws IOException {
    
    reactor = new NioReactor(dispatcher);

    
    LoggingHandler loggingHandler = new LoggingHandler();

    
    reactor.registerChannel(tcpChannel(6666, loggingHandler))
        .registerChannel(tcpChannel(6667, loggingHandler))
        .registerChannel(udpChannel(6668, loggingHandler)).start();
  }

  
  public void stop() throws InterruptedException, IOException {
    reactor.stop();
    dispatcher.stop();
    for (AbstractNioChannel channel : channels) {
      channel.getJavaChannel().close();
    }
  }

  private AbstractNioChannel tcpChannel(int port, ChannelHandler handler) throws IOException {
    NioServerSocketChannel channel = new NioServerSocketChannel(port, handler);
    channel.bind();
    channels.add(channel);
    return channel;
  }

  private AbstractNioChannel udpChannel(int port, ChannelHandler handler) throws IOException {
    NioDatagramChannel channel = new NioDatagramChannel(port, handler);
    channel.bind();
    channels.add(channel);
    return channel;
  }
}
