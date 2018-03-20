

package org.springframework.boot.devtools.tunnel.client;

import java.io.Closeable;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.nio.ByteBuffer;
import java.nio.channels.AsynchronousCloseException;
import java.nio.channels.ServerSocketChannel;
import java.nio.channels.SocketChannel;
import java.nio.channels.WritableByteChannel;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.beans.factory.SmartInitializingSingleton;
import org.springframework.util.Assert;


public class TunnelClient implements SmartInitializingSingleton {

	private static final int BUFFER_SIZE = 1024 * 100;

	private static final Log logger = LogFactory.getLog(TunnelClient.class);

	private final TunnelClientListeners listeners = new TunnelClientListeners();

	private final Object monitor = new Object();

	private final int listenPort;

	private final TunnelConnection tunnelConnection;

	private ServerThread serverThread;

	public TunnelClient(int listenPort, TunnelConnection tunnelConnection) {
		Assert.isTrue(listenPort >= 0, "ListenPort must be greater than or equal to 0");
		Assert.notNull(tunnelConnection, "TunnelConnection must not be null");
		this.listenPort = listenPort;
		this.tunnelConnection = tunnelConnection;
	}

	@Override
	public void afterSingletonsInstantiated() {
		synchronized (this.monitor) {
			if (this.serverThread == null) {
				try {
					start();
				}
				catch (IOException ex) {
					throw new IllegalStateException(ex);
				}
			}
		}
	}

	
	public int start() throws IOException {
		synchronized (this.monitor) {
			Assert.state(this.serverThread == null, "Server already started");
			ServerSocketChannel serverSocketChannel = ServerSocketChannel.open();
			serverSocketChannel.socket().bind(new InetSocketAddress(this.listenPort));
			int port = serverSocketChannel.socket().getLocalPort();
			logger.trace("Listening for TCP traffic to tunnel on port " + port);
			this.serverThread = new ServerThread(serverSocketChannel);
			this.serverThread.start();
			return port;
		}
	}

	
	public void stop() throws IOException {
		synchronized (this.monitor) {
			if (this.serverThread != null) {
				this.serverThread.close();
				try {
					this.serverThread.join(2000);
				}
				catch (InterruptedException ex) {
					Thread.currentThread().interrupt();
				}
				this.serverThread = null;
			}
		}
	}

	protected final ServerThread getServerThread() {
		synchronized (this.monitor) {
			return this.serverThread;
		}
	}

	public void addListener(TunnelClientListener listener) {
		this.listeners.addListener(listener);
	}

	public void removeListener(TunnelClientListener listener) {
		this.listeners.removeListener(listener);
	}

	
	protected class ServerThread extends Thread {

		private final ServerSocketChannel serverSocketChannel;

		private boolean acceptConnections = true;

		public ServerThread(ServerSocketChannel serverSocketChannel) {
			this.serverSocketChannel = serverSocketChannel;
			setName("Tunnel Server");
			setDaemon(true);
		}

		public void close() throws IOException {
			logger.trace("Closing tunnel client on port "
					+ this.serverSocketChannel.socket().getLocalPort());
			this.serverSocketChannel.close();
			this.acceptConnections = false;
			interrupt();
		}

		@Override
		public void run() {
			try {
				while (this.acceptConnections) {
					try (SocketChannel socket = this.serverSocketChannel.accept()) {
						handleConnection(socket);
					}
					catch (AsynchronousCloseException ex) {
											}
				}
			}
			catch (Exception ex) {
				logger.trace("Unexpected exception from tunnel client", ex);
			}
		}

		private void handleConnection(SocketChannel socketChannel) throws Exception {
			Closeable closeable = new SocketCloseable(socketChannel);
			TunnelClient.this.listeners.fireOpenEvent(socketChannel);
			try (WritableByteChannel outputChannel = TunnelClient.this.tunnelConnection
					.open(socketChannel, closeable)) {
				logger.trace("Accepted connection to tunnel client from "
						+ socketChannel.socket().getRemoteSocketAddress());
				while (true) {
					ByteBuffer buffer = ByteBuffer.allocate(BUFFER_SIZE);
					int amountRead = socketChannel.read(buffer);
					if (amountRead == -1) {
						return;
					}
					if (amountRead > 0) {
						buffer.flip();
						outputChannel.write(buffer);
					}
				}
			}
		}

		protected void stopAcceptingConnections() {
			this.acceptConnections = false;
		}

	}

	
	private class SocketCloseable implements Closeable {

		private final SocketChannel socketChannel;

		private boolean closed = false;

		SocketCloseable(SocketChannel socketChannel) {
			this.socketChannel = socketChannel;
		}

		@Override
		public void close() throws IOException {
			if (!this.closed) {
				this.socketChannel.close();
				TunnelClient.this.listeners.fireCloseEvent(this.socketChannel);
				this.closed = true;
			}
		}

	}

}
