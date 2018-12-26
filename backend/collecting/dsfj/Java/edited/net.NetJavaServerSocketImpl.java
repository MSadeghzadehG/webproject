

package com.badlogic.gdx.net;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;

import com.badlogic.gdx.Net.Protocol;
import com.badlogic.gdx.net.ServerSocket;
import com.badlogic.gdx.net.ServerSocketHints;
import com.badlogic.gdx.net.Socket;
import com.badlogic.gdx.net.SocketHints;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class NetJavaServerSocketImpl implements ServerSocket {

	private Protocol protocol;

	
	private java.net.ServerSocket server;

	public NetJavaServerSocketImpl (Protocol protocol, int port, ServerSocketHints hints) {
		this(protocol, null, port, hints);
	}
	
	public NetJavaServerSocketImpl (Protocol protocol, String hostname, int port, ServerSocketHints hints) {
		this.protocol = protocol;

				try {
						server = new java.net.ServerSocket();
			if (hints != null) {
				server.setPerformancePreferences(hints.performancePrefConnectionTime, hints.performancePrefLatency,
					hints.performancePrefBandwidth);
				server.setReuseAddress(hints.reuseAddress);
				server.setSoTimeout(hints.acceptTimeout);
				server.setReceiveBufferSize(hints.receiveBufferSize);
			}

						InetSocketAddress address;
			if( hostname != null ) {
				address = new InetSocketAddress(hostname, port); 
			} else {
				address = new InetSocketAddress(port);
			}
			
			if (hints != null) {
				server.bind(address, hints.backlog);
			} else {
				server.bind(address);
			}
		} catch (Exception e) {
			throw new GdxRuntimeException("Cannot create a server socket at port " + port + ".", e);
		}
	}

	@Override
	public Protocol getProtocol () {
		return protocol;
	}

	@Override
	public Socket accept (SocketHints hints) {
		try {
			return new NetJavaSocketImpl(server.accept(), hints);
		} catch (Exception e) {
			throw new GdxRuntimeException("Error accepting socket.", e);
		}
	}

	@Override
	public void dispose () {
		if (server != null) {
			try {
				server.close();
				server = null;
			} catch (Exception e) {
				throw new GdxRuntimeException("Error closing server.", e);
			}
		}
	}
}
