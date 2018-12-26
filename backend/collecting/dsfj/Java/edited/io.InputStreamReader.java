

package java.io;

import com.badlogic.gdx.utils.Utf8Decoder;

public class InputStreamReader extends Reader {
	private final InputStream in;

	private final Utf8Decoder utf8Decoder;

	public InputStreamReader (InputStream in) {
		this.in = in;
		this.utf8Decoder = new Utf8Decoder();
	}

	public InputStreamReader (InputStream in, String encoding) throws UnsupportedEncodingException {
		this(in);

			}

	public int read (char[] b, int offset, int length) throws IOException {
		byte[] buffer = new byte[length];
		int c = in.read(buffer);
		return c <= 0 ? c : utf8Decoder.decode(buffer, 0, c, b, offset);
	}

	public void close () throws IOException {
		in.close();
	}
}
