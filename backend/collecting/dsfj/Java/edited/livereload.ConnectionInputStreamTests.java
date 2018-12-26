

package org.springframework.boot.devtools.livereload;

import java.io.ByteArrayInputStream;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import static org.assertj.core.api.Assertions.assertThat;


@SuppressWarnings("resource")
public class ConnectionInputStreamTests {

	private static final byte[] NO_BYTES = {};

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	@Test
	public void readHeader() throws Exception {
		String header = "";
		for (int i = 0; i < 100; i++) {
			header += "x-something-" + i
					+ ": xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx";
		}
		String data = header + "\r\n\r\n" + "content\r\n";
		ConnectionInputStream inputStream = new ConnectionInputStream(
				new ByteArrayInputStream(data.getBytes()));
		assertThat(inputStream.readHeader()).isEqualTo(header);
	}

	@Test
	public void readFully() throws Exception {
		byte[] bytes = "the data that we want to read fully".getBytes();
		LimitedInputStream source = new LimitedInputStream(
				new ByteArrayInputStream(bytes), 2);
		ConnectionInputStream inputStream = new ConnectionInputStream(source);
		byte[] buffer = new byte[bytes.length];
		inputStream.readFully(buffer, 0, buffer.length);
		assertThat(buffer).isEqualTo(bytes);
	}

	@Test
	public void checkedRead() throws Exception {
		ConnectionInputStream inputStream = new ConnectionInputStream(
				new ByteArrayInputStream(NO_BYTES));
		this.thrown.expect(IOException.class);
		this.thrown.expectMessage("End of stream");
		inputStream.checkedRead();
	}

	@Test
	public void checkedReadArray() throws Exception {
		ConnectionInputStream inputStream = new ConnectionInputStream(
				new ByteArrayInputStream(NO_BYTES));
		this.thrown.expect(IOException.class);
		this.thrown.expectMessage("End of stream");
		byte[] buffer = new byte[100];
		inputStream.checkedRead(buffer, 0, buffer.length);
	}

	private static class LimitedInputStream extends FilterInputStream {

		private final int max;

		protected LimitedInputStream(InputStream in, int max) {
			super(in);
			this.max = max;
		}

		@Override
		public int read(byte[] b, int off, int len) throws IOException {
			return super.read(b, off, Math.min(len, this.max));
		}

	}

}
