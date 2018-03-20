
package com.iluwatar.reactor.framework;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.util.Iterator;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;


public class NioReactor {

  private static final Logger LOGGER = LoggerFactory.getLogger(NioReactor.class);

  private final Selector selector;
  private final Dispatcher dispatcher;
  
  private final Queue<Runnable> pendingCommands = new ConcurrentLinkedQueue<>();
  private final ExecutorService reactorMain = Executors.newSingleThreadExecutor();

  
  public NioReactor(Dispatcher dispatcher) throws IOException {
    this.dispatcher = dispatcher;
    this.selector = Selector.open();
  }

  
  public void start() throws IOException {
    reactorMain.execute(() -> {
      try {
        LOGGER.info("Reactor started, waiting for events...");
        eventLoop();
      } catch (IOException e) {
        LOGGER.error("exception in event loop", e);
      }
    });
  }

  
  public void stop() throws InterruptedException, IOException {
    reactorMain.shutdownNow();
    selector.wakeup();
    reactorMain.awaitTermination(4, TimeUnit.SECONDS);
    selector.close();
    LOGGER.info("Reactor stopped");
  }

  
  public NioReactor registerChannel(AbstractNioChannel channel) throws IOException {
    SelectionKey key = channel.getJavaChannel().register(selector, channel.getInterestedOps());
    key.attach(channel);
    channel.setReactor(this);
    return this;
  }

  private void eventLoop() throws IOException {
    while (true) {

            if (Thread.interrupted()) {
        break;
      }

            processPendingCommands();

      
      selector.select();

      
      Set<SelectionKey> keys = selector.selectedKeys();

      Iterator<SelectionKey> iterator = keys.iterator();

      while (iterator.hasNext()) {
        SelectionKey key = iterator.next();
        if (!key.isValid()) {
          iterator.remove();
          continue;
        }
        processKey(key);
      }
      keys.clear();
    }
  }

  private void processPendingCommands() {
    Iterator<Runnable> iterator = pendingCommands.iterator();
    while (iterator.hasNext()) {
      Runnable command = iterator.next();
      command.run();
      iterator.remove();
    }
  }

  
  private void processKey(SelectionKey key) throws IOException {
    if (key.isAcceptable()) {
      onChannelAcceptable(key);
    } else if (key.isReadable()) {
      onChannelReadable(key);
    } else if (key.isWritable()) {
      onChannelWritable(key);
    }
  }

  private static void onChannelWritable(SelectionKey key) throws IOException {
    AbstractNioChannel channel = (AbstractNioChannel) key.attachment();
    channel.flush(key);
  }

  private void onChannelReadable(SelectionKey key) {
    try {
            Object readObject = ((AbstractNioChannel) key.attachment()).read(key);

      dispatchReadEvent(key, readObject);
    } catch (IOException e) {
      try {
        key.channel().close();
      } catch (IOException e1) {
        LOGGER.error("error closing channel", e1);
      }
    }
  }

  
  private void dispatchReadEvent(SelectionKey key, Object readObject) {
    dispatcher.onChannelReadEvent((AbstractNioChannel) key.attachment(), readObject, key);
  }

  private void onChannelAcceptable(SelectionKey key) throws IOException {
    ServerSocketChannel serverSocketChannel = (ServerSocketChannel) key.channel();
    SocketChannel socketChannel = serverSocketChannel.accept();
    socketChannel.configureBlocking(false);
    SelectionKey readKey = socketChannel.register(selector, SelectionKey.OP_READ);
    readKey.attach(key.attachment());
  }

  
  public void changeOps(SelectionKey key, int interestedOps) {
    pendingCommands.add(new ChangeKeyOpsCommand(key, interestedOps));
    selector.wakeup();
  }

  
  class ChangeKeyOpsCommand implements Runnable {
    private SelectionKey key;
    private int interestedOps;

    public ChangeKeyOpsCommand(SelectionKey key, int interestedOps) {
      this.key = key;
      this.interestedOps = interestedOps;
    }

    public void run() {
      key.interestOps(interestedOps);
    }

    @Override
    public String toString() {
      return "Change of ops to: " + interestedOps;
    }
  }
}
