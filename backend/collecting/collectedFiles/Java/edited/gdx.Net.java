

package com.badlogic.gdx;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.badlogic.gdx.Application.ApplicationType;
import com.badlogic.gdx.net.HttpRequestHeader;
import com.badlogic.gdx.net.HttpResponseHeader;
import com.badlogic.gdx.net.HttpStatus;
import com.badlogic.gdx.net.ServerSocket;
import com.badlogic.gdx.net.ServerSocketHints;
import com.badlogic.gdx.net.Socket;
import com.badlogic.gdx.net.SocketHints;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.Pool.Poolable;


public interface Net {

	
	public static interface HttpResponse {
		
		byte[] getResult ();

		
		String getResultAsString ();

		
		InputStream getResultAsStream ();

		
		HttpStatus getStatus ();

		
		String getHeader (String name);

		
		Map<String, List<String>> getHeaders ();
	}

	
	public static interface HttpMethods {

		public static final String GET = "GET";
		public static final String POST = "POST";
		public static final String PUT = "PUT";
		public static final String DELETE = "DELETE";

	}

	
	public static class HttpRequest implements Poolable {

		private String httpMethod;
		private String url;
		private Map<String, String> headers;
		private int timeOut = 0;

		private String content;
		private InputStream contentStream;
		private long contentLength;

		private boolean followRedirects = true;

		private boolean includeCredentials = false;
		
		public HttpRequest () {
			this.headers = new HashMap<String, String>();
		}

		
		public HttpRequest (String httpMethod) {
			this();
			this.httpMethod = httpMethod;
		}

		
		public void setUrl (String url) {
			this.url = url;
		}

		
		public void setHeader (String name, String value) {
			headers.put(name, value);
		}

		
		public void setContent (String content) {
			this.content = content;
		}

		
		public void setContent (InputStream contentStream, long contentLength) {
			this.contentStream = contentStream;
			this.contentLength = contentLength;
		}

		
		public void setTimeOut (int timeOut) {
			this.timeOut = timeOut;
		}

		
		public void setFollowRedirects (boolean followRedirects) throws IllegalArgumentException {
			if (followRedirects == true || Gdx.app.getType() != ApplicationType.WebGL) {
				this.followRedirects = followRedirects;
			} else {
				throw new IllegalArgumentException("Following redirects can't be disabled using the GWT/WebGL backend!");
			}
		}

		
		public void setIncludeCredentials (boolean includeCredentials) {
			this.includeCredentials = includeCredentials;
		}
		
		
		public void setMethod (String httpMethod) {
			this.httpMethod = httpMethod;
		}

		
		public int getTimeOut () {
			return timeOut;
		}

		
		public String getMethod () {
			return httpMethod;
		}

		
		public String getUrl () {
			return url;
		}

		
		public String getContent () {
			return content;
		}

		
		public InputStream getContentStream () {
			return contentStream;
		}

		
		public long getContentLength () {
			return contentLength;
		}

		
		public Map<String, String> getHeaders () {
			return headers;
		}

		
		public boolean getFollowRedirects () {
			return followRedirects;
		}
		
		
		public boolean getIncludeCredentials () {
			return includeCredentials;
		}

		@Override
		public void reset () {
			httpMethod = null;
			url = null;
			headers.clear();
			timeOut = 0;

			content = null;
			contentStream = null;
			contentLength = 0;

			followRedirects = true;
		}

	}

	
	public static interface HttpResponseListener {

		
		void handleHttpResponse (HttpResponse httpResponse);

		
		void failed (Throwable t);

		void cancelled ();
	}

	
	public void sendHttpRequest (HttpRequest httpRequest, HttpResponseListener httpResponseListener);

	public void cancelHttpRequest (HttpRequest httpRequest);

	
	public enum Protocol {
		TCP
	}
	
	
	public ServerSocket newServerSocket (Protocol protocol, String hostname, int port, ServerSocketHints hints);

	
	public ServerSocket newServerSocket (Protocol protocol, int port, ServerSocketHints hints);

	
	public Socket newClientSocket (Protocol protocol, String host, int port, SocketHints hints);

	
	public boolean openURI (String URI);
}
