

package org.springframework.boot.devtools.restart.classloader;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.net.URLConnection;
import java.net.URLStreamHandler;


public class ClassLoaderFileURLStreamHandler extends URLStreamHandler {

	private ClassLoaderFile file;

	public ClassLoaderFileURLStreamHandler(ClassLoaderFile file) {
		this.file = file;
	}

	@Override
	protected URLConnection openConnection(URL url) throws IOException {
		return new Connection(url);
	}

	private class Connection extends URLConnection {

		Connection(URL url) {
			super(url);
		}

		@Override
		public void connect() throws IOException {
		}

		@Override
		public InputStream getInputStream() throws IOException {
			return new ByteArrayInputStream(
					ClassLoaderFileURLStreamHandler.this.file.getContents());
		}

		@Override
		public long getLastModified() {
			return ClassLoaderFileURLStreamHandler.this.file.getLastModified();

		}

	}

}
