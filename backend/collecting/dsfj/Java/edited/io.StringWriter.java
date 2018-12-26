

package java.io;

public class StringWriter extends Writer {
	private final StringBuilder out;

	public StringWriter() {
		out = new StringBuilder();
	}
	
	public StringWriter(int initialCapacity) {
		out = new StringBuilder(initialCapacity);
	}
	
	public void write (char[] b, int offset, int length) throws IOException {
		out.append(b, offset, length);
	}

	public String toString () {
		return out.toString();
	}

	public void flush () throws IOException {
	}

	public void close () throws IOException {
	}
}
