

package java.io;

import avian.Utf8;

public class OutputStreamWriter extends Writer {
	private final OutputStream out;

	public OutputStreamWriter (OutputStream out, String encoding) {
		this(out);
	}

	public OutputStreamWriter (OutputStream out) {
		this.out = out;
	}

	public void write (char[] b, int offset, int length) throws IOException {
		out.write(Utf8.encode(b, offset, length));
	}

	public void flush () throws IOException {
		out.flush();
	}

	public void close () throws IOException {
		out.close();
	}
}
