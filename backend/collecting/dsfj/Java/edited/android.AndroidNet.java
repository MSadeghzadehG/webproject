

package com.badlogic.gdx.backends.android;

import android.app.Activity;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;

import com.badlogic.gdx.Net;
import com.badlogic.gdx.net.NetJavaImpl;
import com.badlogic.gdx.net.NetJavaServerSocketImpl;
import com.badlogic.gdx.net.NetJavaSocketImpl;
import com.badlogic.gdx.net.ServerSocket;
import com.badlogic.gdx.net.ServerSocketHints;
import com.badlogic.gdx.net.Socket;
import com.badlogic.gdx.net.SocketHints;
import com.badlogic.gdx.utils.GdxRuntimeException;


public class AndroidNet implements Net {

			final AndroidApplicationBase app;
	NetJavaImpl netJavaImpl;

	public AndroidNet (AndroidApplicationBase app) {
		this.app = app;
		netJavaImpl = new NetJavaImpl();
	}

	@Override
	public void sendHttpRequest (HttpRequest httpRequest, final HttpResponseListener httpResponseListener) {
		netJavaImpl.sendHttpRequest(httpRequest, httpResponseListener);
	}

	@Override
	public void cancelHttpRequest (HttpRequest httpRequest) {
		netJavaImpl.cancelHttpRequest(httpRequest);
	}
	
	@Override
	public ServerSocket newServerSocket (Protocol protocol, String hostname, int port, ServerSocketHints hints) {
		return new NetJavaServerSocketImpl(protocol, hostname, port, hints);
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
		boolean result = false;
		final Uri uri = Uri.parse(URI);
		Intent intent = new Intent(Intent.ACTION_VIEW, uri);
		PackageManager pm = app.getContext().getPackageManager();
		if (pm.resolveActivity(intent, PackageManager.MATCH_DEFAULT_ONLY) != null) {
			app.runOnUiThread(new Runnable() {
				@Override
				public void run () {
					Intent intent = new Intent(Intent.ACTION_VIEW, uri);
										if (!(app.getContext() instanceof Activity))
						intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
					app.startActivity(intent);
				}
			});
			result = true;
		}
		return result;
	}

}
