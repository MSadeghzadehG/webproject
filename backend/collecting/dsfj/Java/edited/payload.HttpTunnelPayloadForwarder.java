

package org.springframework.boot.devtools.tunnel.payload;

import java.io.IOException;
import java.nio.channels.WritableByteChannel;
import java.util.HashMap;
import java.util.Map;

import org.springframework.util.Assert;


public class HttpTunnelPayloadForwarder {

	private static final int MAXIMUM_QUEUE_SIZE = 100;

	private final Map<Long, HttpTunnelPayload> queue = new HashMap<>();

	private final Object monitor = new Object();

	private final WritableByteChannel targetChannel;

	private long lastRequestSeq = 0;

	
	public HttpTunnelPayloadForwarder(WritableByteChannel targetChannel) {
		Assert.notNull(targetChannel, "TargetChannel must not be null");
		this.targetChannel = targetChannel;
	}

	public void forward(HttpTunnelPayload payload) throws IOException {
		synchronized (this.monitor) {
			long seq = payload.getSequence();
			if (this.lastRequestSeq != seq - 1) {
				Assert.state(this.queue.size() < MAXIMUM_QUEUE_SIZE,
						"Too many messages queued");
				this.queue.put(seq, payload);
				return;
			}
			payload.logOutgoing();
			payload.writeTo(this.targetChannel);
			this.lastRequestSeq = seq;
			HttpTunnelPayload queuedItem = this.queue.get(seq + 1);
			if (queuedItem != null) {
				forward(queuedItem);
			}
		}
	}

}
