

package org.springframework.boot.web.embedded.netty;

import java.net.InetSocketAddress;
import java.time.Duration;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import reactor.ipc.netty.http.server.HttpServer;
import reactor.ipc.netty.http.server.HttpServerOptions.Builder;

import org.springframework.boot.web.reactive.server.AbstractReactiveWebServerFactory;
import org.springframework.boot.web.reactive.server.ReactiveWebServerFactory;
import org.springframework.boot.web.server.WebServer;
import org.springframework.http.server.reactive.HttpHandler;
import org.springframework.http.server.reactive.ReactorHttpHandlerAdapter;
import org.springframework.util.Assert;


public class NettyReactiveWebServerFactory extends AbstractReactiveWebServerFactory {

	private List<NettyServerCustomizer> serverCustomizers = new ArrayList<>();

	private Duration lifecycleTimeout;

	public NettyReactiveWebServerFactory() {
	}

	public NettyReactiveWebServerFactory(int port) {
		super(port);
	}

	@Override
	public WebServer getWebServer(HttpHandler httpHandler) {
		HttpServer httpServer = createHttpServer();
		ReactorHttpHandlerAdapter handlerAdapter = new ReactorHttpHandlerAdapter(
				httpHandler);
		return new NettyWebServer(httpServer, handlerAdapter, this.lifecycleTimeout);
	}

	
	public Collection<NettyServerCustomizer> getServerCustomizers() {
		return this.serverCustomizers;
	}

	
	public void setServerCustomizers(
			Collection<? extends NettyServerCustomizer> serverCustomizers) {
		Assert.notNull(serverCustomizers, "ServerCustomizers must not be null");
		this.serverCustomizers = new ArrayList<>(serverCustomizers);
	}

	
	public void addServerCustomizers(NettyServerCustomizer... serverCustomizers) {
		Assert.notNull(serverCustomizers, "ServerCustomizer must not be null");
		this.serverCustomizers.addAll(Arrays.asList(serverCustomizers));
	}

	
	public void setLifecycleTimeout(Duration lifecycleTimeout) {
		this.lifecycleTimeout = lifecycleTimeout;
	}

	private HttpServer createHttpServer() {
		return HttpServer.builder().options((options) -> {
			options.listenAddress(getListenAddress());
			if (getSsl() != null && getSsl().isEnabled()) {
				SslServerCustomizer sslServerCustomizer = new SslServerCustomizer(
						getSsl(), getSslStoreProvider());
				sslServerCustomizer.customize(options);
			}
			if (getCompression() != null && getCompression().getEnabled()) {
				CompressionCustomizer compressionCustomizer = new CompressionCustomizer(
						getCompression());
				compressionCustomizer.customize(options);
			}
			applyCustomizers(options);
		}).build();
	}

	private InetSocketAddress getListenAddress() {
		if (getAddress() != null) {
			return new InetSocketAddress(getAddress().getHostAddress(), getPort());
		}
		return new InetSocketAddress(getPort());
	}

	private void applyCustomizers(Builder options) {
		this.serverCustomizers.forEach((customizer) -> customizer.customize(options));
	}

}
