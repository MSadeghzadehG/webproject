

package org.springframework.boot.devtools.tunnel.payload;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.nio.channels.Channels;
import java.nio.channels.WritableByteChannel;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import static org.assertj.core.api.Assertions.assertThat;


public class HttpTunnelPayloadForwarderTests {

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	@Test
	public void targetChannelMustNotBeNull() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("TargetChannel must not be null");
		new HttpTunnelPayloadForwarder(null);
	}

	@Test
	public void forwardInSequence() throws Exception {
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		WritableByteChannel channel = Channels.newChannel(out);
		HttpTunnelPayloadForwarder forwarder = new HttpTunnelPayloadForwarder(channel);
		forwarder.forward(payload(1, "he"));
		forwarder.forward(payload(2, "ll"));
		forwarder.forward(payload(3, "o"));
		assertThat(out.toByteArray()).isEqualTo("hello".getBytes());
	}

	@Test
	public void forwardOutOfSequence() throws Exception {
		ByteArrayOutputStream out = new ByteArrayOutputStream();
		WritableByteChannel channel = Channels.newChannel(out);
		HttpTunnelPayloadForwarder forwarder = new HttpTunnelPayloadForwarder(channel);
		forwarder.forward(payload(3, "o"));
		forwarder.forward(payload(2, "ll"));
		forwarder.forward(payload(1, "he"));
		assertThat(out.toByteArray()).isEqualTo("hello".getBytes());
	}

	@Test
	public void overflow() throws Exception {
		WritableByteChannel channel = Channels.newChannel(new ByteArrayOutputStream());
		HttpTunnelPayloadForwarder forwarder = new HttpTunnelPayloadForwarder(channel);
		this.thrown.expect(IllegalStateException.class);
		this.thrown.expectMessage("Too many messages queued");
		for (int i = 2; i < 130; i++) {
			forwarder.forward(payload(i, "data" + i));
		}
	}

	private HttpTunnelPayload payload(long sequence, String data) {
		return new HttpTunnelPayload(sequence, ByteBuffer.wrap(data.getBytes()));
	}

}
