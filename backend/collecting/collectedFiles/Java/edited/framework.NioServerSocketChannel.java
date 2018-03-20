
package com.iluwatar.reactor.framework;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;


public class NioServerSocketChannel extends AbstractNioChannel {

  private static final Logger LOGGER = LoggerFactory.getLogger(NioServerSocketChannel.class);

  private final int port;

  
  public NioServerSocketChannel(int port, ChannelHandler handler) throws IOException {
    super(handler, ServerSocketChannel.open());
    this.port = port;
  }


  @Override
  public int getInterestedOps() {
        return SelectionKey.OP_ACCEPT;
  }

  
  @Override
  public ServerSocketChannel getJavaChannel() {
    return (ServerSocketChannel) super.getJavaChannel();
  }

  
  @Override
  public ByteBuffer read(SelectionKey key) throws IOException {
    SocketChannel socketChannel = (SocketChannel) key.channel();
    ByteBuffer buffer = ByteBuffer.allocate(1024);
    int read = socketChannel.read(buffer);
    buffer.flip();
    if (read == -1) {
      throw new IOException("Socket closed");
    }
    return buffer;
  }

  
  @Override
  public void bind() throws IOException {
    ((ServerSocketChannel) getJavaChannel()).socket().bind(
        new InetSocketAddress(InetAddress.getLocalHost(), port));
    ((ServerSocketChannel) getJavaChannel()).configureBlocking(false);
    LOGGER.info("Bound TCP socket at port: {}", port);
  }

  
  @Override
  protected void doWrite(Object pendingWrite, SelectionKey key) throws IOException {
    ByteBuffer pendingBuffer = (ByteBuffer) pendingWrite;
    ((SocketChannel) key.channel()).write(pendingBuffer);
  }
}
