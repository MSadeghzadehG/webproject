

package org.springframework.boot.devtools.livereload;

import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;


class ConnectionInputStream extends FilterInputStream {

	private static final String HEADER_END = "\r\n\r\n";

	private static final int BUFFER_SIZE = 4096;

	ConnectionInputStream(InputStream in) {
		super(in);
	}

	
	public String readHeader() throws IOException {
		byte[] buffer = new byte[BUFFER_SIZE];
		StringBuilder content = new StringBuilder(BUFFER_SIZE);
		while (content.indexOf(HEADER_END) == -1) {
			int amountRead = checkedRead(buffer, 0, BUFFER_SIZE);
			content.append(new String(buffer, 0, amountRead));
		}
		return content.substring(0, content.indexOf(HEADER_END));
	}

	
	public void readFully(byte[] buffer, int offset, int length) throws IOException {
		while (length > 0) {
			int amountRead = checkedRead(buffer, offset, length);
			offset += amountRead;
			length -= amountRead;
		}
	}

	
	public int checkedRead() throws IOException {
		int b = read();
		if (b == -1) {
			throw new IOException("End of stream");
		}
		return (b & 0xff);
	}

	
	public int checkedRead(byte[] buffer, int offset, int length) throws IOException {
		int amountRead = read(buffer, offset, length);
		if (amountRead == -1) {
			throw new IOException("End of stream");
		}
		return amountRead;
	}

}
