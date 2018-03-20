

package org.springframework.boot.devtools.tunnel.client;

import java.nio.channels.SocketChannel;
import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import org.springframework.util.Assert;


class TunnelClientListeners {

	private final List<TunnelClientListener> listeners = new CopyOnWriteArrayList<>();

	public void addListener(TunnelClientListener listener) {
		Assert.notNull(listener, "Listener must not be null");
		this.listeners.add(listener);
	}

	public void removeListener(TunnelClientListener listener) {
		Assert.notNull(listener, "Listener must not be null");
		this.listeners.remove(listener);
	}

	public void fireOpenEvent(SocketChannel socket) {
		for (TunnelClientListener listener : this.listeners) {
			listener.onOpen(socket);
		}
	}

	public void fireCloseEvent(SocketChannel socket) {
		for (TunnelClientListener listener : this.listeners) {
			listener.onClose(socket);
		}
	}

}
