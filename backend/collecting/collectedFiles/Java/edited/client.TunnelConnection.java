

package org.springframework.boot.devtools.tunnel.client;

import java.io.Closeable;
import java.nio.channels.WritableByteChannel;


@FunctionalInterface
public interface TunnelConnection {

	
	WritableByteChannel open(WritableByteChannel incomingChannel, Closeable closeable)
			throws Exception;

}
