

package com.badlogic.gdx.net;

import java.io.InputStream;
import java.io.OutputStream;

import com.badlogic.gdx.Net;
import com.badlogic.gdx.Net.Protocol;
import com.badlogic.gdx.utils.Disposable;


public interface Socket extends Disposable {
	
	public boolean isConnected ();

	
	public InputStream getInputStream ();

	
	public OutputStream getOutputStream ();

	
	public String getRemoteAddress ();
}
