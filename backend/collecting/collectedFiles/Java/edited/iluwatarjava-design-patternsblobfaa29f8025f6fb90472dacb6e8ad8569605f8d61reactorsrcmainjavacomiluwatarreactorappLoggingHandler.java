
package com.iluwatar.reactor.app;

import java.nio.ByteBuffer;
import java.nio.channels.SelectionKey;

import com.iluwatar.reactor.framework.AbstractNioChannel;
import com.iluwatar.reactor.framework.ChannelHandler;
import com.iluwatar.reactor.framework.NioDatagramChannel.DatagramPacket;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;


public class LoggingHandler implements ChannelHandler {

  private static final Logger LOGGER = LoggerFactory.getLogger(LoggingHandler.class);

  private static final byte[] ACK = "Data logged successfully".getBytes();

  
  @Override
  public void handleChannelRead(AbstractNioChannel channel, Object readObject, SelectionKey key) {
    
    if (readObject instanceof ByteBuffer) {
      doLogging((ByteBuffer) readObject);
      sendReply(channel, key);
    } else if (readObject instanceof DatagramPacket) {
      DatagramPacket datagram = (DatagramPacket) readObject;
      doLogging(datagram.getData());
      sendReply(channel, datagram, key);
    } else {
      throw new IllegalStateException("Unknown data received");
    }
  }

  private static void sendReply(AbstractNioChannel channel, DatagramPacket incomingPacket, SelectionKey key) {
    
    DatagramPacket replyPacket = new DatagramPacket(ByteBuffer.wrap(ACK));
    replyPacket.setReceiver(incomingPacket.getSender());

    channel.write(replyPacket, key);
  }

  private static void sendReply(AbstractNioChannel channel, SelectionKey key) {
    ByteBuffer buffer = ByteBuffer.wrap(ACK);
    channel.write(buffer, key);
  }

  private static void doLogging(ByteBuffer data) {
        LOGGER.info(new String(data.array(), 0, data.limit()));
  }
}
