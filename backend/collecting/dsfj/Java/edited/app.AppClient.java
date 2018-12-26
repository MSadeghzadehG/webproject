
package com.iluwatar.reactor.app;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.PrintWriter;
import java.net.DatagramPacket;
import java.net.DatagramSocket;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;


public class AppClient {

  private static final Logger LOGGER = LoggerFactory.getLogger(AppClient.class);

  private final ExecutorService service = Executors.newFixedThreadPool(4);

  
  public static void main(String[] args) throws IOException {
    AppClient appClient = new AppClient();
    appClient.start();
  }

  
  public void start() throws IOException {
    LOGGER.info("Starting logging clients");
    service.execute(new TcpLoggingClient("Client 1", 6666));
    service.execute(new TcpLoggingClient("Client 2", 6667));
    service.execute(new UdpLoggingClient("Client 3", 6668));
    service.execute(new UdpLoggingClient("Client 4", 6668));
  }

  
  public void stop() {
    service.shutdown();
    if (!service.isTerminated()) {
      service.shutdownNow();
      try {
        service.awaitTermination(1000, TimeUnit.SECONDS);
      } catch (InterruptedException e) {
        LOGGER.error("exception awaiting termination", e);
      }
    }
    LOGGER.info("Logging clients stopped");
  }

  private static void artificialDelayOf(long millis) {
    try {
      Thread.sleep(millis);
    } catch (InterruptedException e) {
      LOGGER.error("sleep interrupted", e);
    }
  }

  
  static class TcpLoggingClient implements Runnable {

    private final int serverPort;
    private final String clientName;

    
    public TcpLoggingClient(String clientName, int serverPort) {
      this.clientName = clientName;
      this.serverPort = serverPort;
    }

    public void run() {
      try (Socket socket = new Socket(InetAddress.getLocalHost(), serverPort)) {
        OutputStream outputStream = socket.getOutputStream();
        PrintWriter writer = new PrintWriter(outputStream);
        sendLogRequests(writer, socket.getInputStream());
      } catch (IOException e) {
        LOGGER.error("error sending requests", e);
        throw new RuntimeException(e);
      }
    }

    private void sendLogRequests(PrintWriter writer, InputStream inputStream) throws IOException {
      for (int i = 0; i < 4; i++) {
        writer.println(clientName + " - Log request: " + i);
        writer.flush();

        byte[] data = new byte[1024];
        int read = inputStream.read(data, 0, data.length);
        if (read == 0) {
          LOGGER.info("Read zero bytes");
        } else {
          LOGGER.info(new String(data, 0, read));
        }

        artificialDelayOf(100);
      }
    }

  }

  
  static class UdpLoggingClient implements Runnable {
    private final String clientName;
    private final InetSocketAddress remoteAddress;

    
    public UdpLoggingClient(String clientName, int port) throws UnknownHostException {
      this.clientName = clientName;
      this.remoteAddress = new InetSocketAddress(InetAddress.getLocalHost(), port);
    }

    @Override
    public void run() {
      try (DatagramSocket socket = new DatagramSocket()) {
        for (int i = 0; i < 4; i++) {

          String message = clientName + " - Log request: " + i;
          DatagramPacket request =
              new DatagramPacket(message.getBytes(), message.getBytes().length, remoteAddress);

          socket.send(request);

          byte[] data = new byte[1024];
          DatagramPacket reply = new DatagramPacket(data, data.length);
          socket.receive(reply);
          if (reply.getLength() == 0) {
            LOGGER.info("Read zero bytes");
          } else {
            LOGGER.info(new String(reply.getData(), 0, reply.getLength()));
          }

          artificialDelayOf(100);
        }
      } catch (IOException e1) {
        LOGGER.error("error sending packets", e1);
      }
    }
  }
}
