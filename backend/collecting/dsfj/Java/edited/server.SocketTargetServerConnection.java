

package org.springframework.boot.devtools.tunnel.server;

import java.io.IOException;
import java.net.InetSocketAddress;
import java.net.SocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.ByteChannel;
import java.nio.channels.Channels;
import java.nio.channels.ReadableByteChannel;
import java.nio.channels.SocketChannel;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.util.Assert;


public class SocketTargetServerConnection implements TargetServerConnection {

	private static final Log logger = LogFactory
			.getLog(SocketTargetServerConnection.class);

	private final PortProvider portProvider;

	
	public SocketTargetServerConnection(PortProvider portProvider) {
		Assert.notNull(portProvider, "PortProvider must not be null");
		this.portProvider = portProvider;
	}

	@Override
	public ByteChannel open(int socketTimeout) throws IOException {
		SocketAddress address = new InetSocketAddress(this.portProvider.getPort());
		logger.trace("Opening tunnel connection to target server on " + address);
		SocketChannel channel = SocketChannel.open(address);
		channel.socket().setSoTimeout(socketTimeout);
		return new TimeoutAwareChannel(channel);
	}

	
	private static class TimeoutAwareChannel implements ByteChannel {

		private final SocketChannel socketChannel;

		private final ReadableByteChannel readChannel;

		TimeoutAwareChannel(SocketChannel socketChannel) throws IOException {
			this.socketChannel = socketChannel;
			this.readChannel = Channels
					.newChannel(socketChannel.socket().getInputStream());
		}

		@Override
		public int read(ByteBuffer dst) throws IOException {
			return this.readChannel.read(dst);
		}

		@Override
		public int write(ByteBuffer src) throws IOException {
			return this.socketChannel.write(src);
		}

		@Override
		public boolean isOpen() {
			return this.socketChannel.isOpen();
		}

		@Override
		public void close() throws IOException {
			this.socketChannel.close();
		}

	}

}
