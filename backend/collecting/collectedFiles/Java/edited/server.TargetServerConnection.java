

package org.springframework.boot.devtools.tunnel.server;

import java.io.IOException;
import java.nio.channels.ByteChannel;


@FunctionalInterface
public interface TargetServerConnection {

	
	ByteChannel open(int timeout) throws IOException;

}
