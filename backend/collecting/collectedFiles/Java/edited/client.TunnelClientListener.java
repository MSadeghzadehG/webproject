

package org.springframework.boot.devtools.tunnel.client;

import java.nio.channels.SocketChannel;


public interface TunnelClientListener {

	
	void onOpen(SocketChannel socket);

	
	void onClose(SocketChannel socket);

}
