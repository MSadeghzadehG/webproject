

package com.badlogic.gdx.backends.lwjgl3;

import java.awt.Desktop;
import java.io.IOException;
import java.net.URI;

import com.apple.eio.FileManager;
import com.badlogic.gdx.Net;
import com.badlogic.gdx.net.NetJavaImpl;
import com.badlogic.gdx.net.NetJavaServerSocketImpl;
import com.badlogic.gdx.net.NetJavaSocketImpl;
import com.badlogic.gdx.net.ServerSocket;
import com.badlogic.gdx.net.ServerSocketHints;
import com.badlogic.gdx.net.Socket;
import com.badlogic.gdx.net.SocketHints;
import com.badlogic.gdx.utils.SharedLibraryLoader;


public class Lwjgl3Net implements Net {

	NetJavaImpl netJavaImpl = new NetJavaImpl();

	@Override
	public void sendHttpRequest (HttpRequest httpRequest, HttpResponseListener httpResponseListener) {
		netJavaImpl.sendHttpRequest(httpRequest, httpResponseListener);
	}
	
	@Override
	public void cancelHttpRequest (HttpRequest httpRequest) {
		netJavaImpl.cancelHttpRequest(httpRequest);
	}
	
	@Override
	public ServerSocket newServerSocket (Protocol protocol, String ipAddress, int port, ServerSocketHints hints) {
		return new NetJavaServerSocketImpl(protocol, ipAddress, port, hints);
	}

	@Override
	public ServerSocket newServerSocket (Protocol protocol, int port, ServerSocketHints hints) {
		return new NetJavaServerSocketImpl(protocol, port, hints);
	}

	@Override
	public Socket newClientSocket (Protocol protocol, String host, int port, SocketHints hints) {
		return new NetJavaSocketImpl(protocol, host, port, hints);
	}

	@Override
	public boolean openURI (String URI) {
		if(SharedLibraryLoader.isMac) {
			try {
				FileManager.openURL(URI);
				return true;
			} catch (IOException e) {
				return false;
			}
		} else {
			try {
				Desktop.getDesktop().browse(new URI(URI));
				return true;
			} catch (Throwable t) {
				return false;
			}
		}
	}

}
