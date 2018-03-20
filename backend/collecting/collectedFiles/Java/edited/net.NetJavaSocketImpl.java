

package com.badlogic.gdx.net;

import java.io.InputStream;
import java.io.OutputStream;
import java.net.InetSocketAddress;

import com.badlogic.gdx.Net.Protocol;
import com.badlogic.gdx.net.ServerSocketHints;
import com.badlogic.gdx.net.Socket;
import com.badlogic.gdx.net.SocketHints;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class NetJavaSocketImpl implements Socket {

	
	private java.net.Socket socket;

	public NetJavaSocketImpl (Protocol protocol, String host, int port, SocketHints hints) {
		try {
						socket = new java.net.Socket();
			applyHints(hints); 
						InetSocketAddress address = new InetSocketAddress(host, port);
			if (hints != null) {
				socket.connect(address, hints.connectTimeout);
			} else {
				socket.connect(address);
			}
		} catch (Exception e) {
			throw new GdxRuntimeException("Error making a socket connection to " + host + ":" + port, e);
		}
	}

	public NetJavaSocketImpl (java.net.Socket socket, SocketHints hints) {
		this.socket = socket;
		applyHints(hints);
	}

	private void applyHints (SocketHints hints) {
		if (hints != null) {
			try {
				socket.setPerformancePreferences(hints.performancePrefConnectionTime, hints.performancePrefLatency,
					hints.performancePrefBandwidth);
				socket.setTrafficClass(hints.trafficClass);
				socket.setTcpNoDelay(hints.tcpNoDelay);
				socket.setKeepAlive(hints.keepAlive);
				socket.setSendBufferSize(hints.sendBufferSize);
				socket.setReceiveBufferSize(hints.receiveBufferSize);
				socket.setSoLinger(hints.linger, hints.lingerDuration);
				socket.setSoTimeout(hints.socketTimeout);
			} catch (Exception e) {
				throw new GdxRuntimeException("Error setting socket hints.", e);
			}
		}
	}

	@Override
	public boolean isConnected () {
		if (socket != null) {
			return socket.isConnected();
		} else {
			return false;
		}
	}

	@Override
	public InputStream getInputStream () {
		try {
			return socket.getInputStream();
		} catch (Exception e) {
			throw new GdxRuntimeException("Error getting input stream from socket.", e);
		}
	}

	@Override
	public OutputStream getOutputStream () {
		try {
			return socket.getOutputStream();
		} catch (Exception e) {
			throw new GdxRuntimeException("Error getting output stream from socket.", e);
		}
	}

	@Override
	public String getRemoteAddress () {
		return socket.getRemoteSocketAddress().toString();
	}

	@Override
	public void dispose () {
		if (socket != null) {
			try {
				socket.close();
				socket = null;
			} catch (Exception e) {
				throw new GdxRuntimeException("Error closing socket.", e);
			}
		}
	}
}
