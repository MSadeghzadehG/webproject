

package org.springframework.boot.web.embedded.jetty;

import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.eclipse.jetty.server.AbstractConnector;
import org.eclipse.jetty.server.ConnectionFactory;
import org.eclipse.jetty.server.Handler;
import org.eclipse.jetty.server.HttpConfiguration;
import org.eclipse.jetty.server.Server;
import org.eclipse.jetty.server.ServerConnector;
import org.eclipse.jetty.server.handler.HandlerWrapper;
import org.eclipse.jetty.servlet.ServletContextHandler;
import org.eclipse.jetty.servlet.ServletHolder;
import org.eclipse.jetty.util.thread.ThreadPool;

import org.springframework.boot.web.reactive.server.AbstractReactiveWebServerFactory;
import org.springframework.boot.web.reactive.server.ReactiveWebServerFactory;
import org.springframework.boot.web.server.WebServer;
import org.springframework.http.server.reactive.HttpHandler;
import org.springframework.http.server.reactive.JettyHttpHandlerAdapter;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


public class JettyReactiveWebServerFactory extends AbstractReactiveWebServerFactory
		implements ConfigurableJettyWebServerFactory {

	private static final Log logger = LogFactory
			.getLog(JettyReactiveWebServerFactory.class);

	
	private int acceptors = -1;

	
	private int selectors = -1;

	private boolean useForwardHeaders;

	private List<JettyServerCustomizer> jettyServerCustomizers = new ArrayList<>();

	private ThreadPool threadPool;

	
	public JettyReactiveWebServerFactory() {
	}

	
	public JettyReactiveWebServerFactory(int port) {
		super(port);
	}

	@Override
	public void setUseForwardHeaders(boolean useForwardHeaders) {
		this.useForwardHeaders = useForwardHeaders;
	}

	@Override
	public void setAcceptors(int acceptors) {
		this.acceptors = acceptors;
	}

	@Override
	public WebServer getWebServer(HttpHandler httpHandler) {
		JettyHttpHandlerAdapter servlet = new JettyHttpHandlerAdapter(httpHandler);
		Server server = createJettyServer(servlet);
		return new JettyWebServer(server, getPort() >= 0);
	}

	@Override
	public void addServerCustomizers(JettyServerCustomizer... customizers) {
		Assert.notNull(customizers, "Customizers must not be null");
		this.jettyServerCustomizers.addAll(Arrays.asList(customizers));
	}

	
	public void setServerCustomizers(
			Collection<? extends JettyServerCustomizer> customizers) {
		Assert.notNull(customizers, "Customizers must not be null");
		this.jettyServerCustomizers = new ArrayList<>(customizers);
	}

	
	public Collection<JettyServerCustomizer> getServerCustomizers() {
		return this.jettyServerCustomizers;
	}

	@Override
	public void setSelectors(int selectors) {
		this.selectors = selectors;
	}

	protected Server createJettyServer(JettyHttpHandlerAdapter servlet) {
		int port = (getPort() >= 0 ? getPort() : 0);
		InetSocketAddress address = new InetSocketAddress(getAddress(), port);
		Server server = new Server(getThreadPool());
		server.addConnector(createConnector(address, server));
		ServletHolder servletHolder = new ServletHolder(servlet);
		servletHolder.setAsyncSupported(true);
		ServletContextHandler contextHandler = new ServletContextHandler(server, "",
				false, false);
		contextHandler.addServlet(servletHolder, "/");
		server.setHandler(addHandlerWrappers(contextHandler));
		JettyReactiveWebServerFactory.logger
				.info("Server initialized with port: " + port);
		if (getSsl() != null && getSsl().isEnabled()) {
			customizeSsl(server, address);
		}
		for (JettyServerCustomizer customizer : getServerCustomizers()) {
			customizer.customize(server);
		}
		if (this.useForwardHeaders) {
			new ForwardHeadersCustomizer().customize(server);
		}
		return server;
	}

	private AbstractConnector createConnector(InetSocketAddress address, Server server) {
		ServerConnector connector = new ServerConnector(server, this.acceptors,
				this.selectors);
		connector.setHost(address.getHostString());
		connector.setPort(address.getPort());
		for (ConnectionFactory connectionFactory : connector.getConnectionFactories()) {
			if (connectionFactory instanceof HttpConfiguration.ConnectionFactory) {
				((HttpConfiguration.ConnectionFactory) connectionFactory)
						.getHttpConfiguration().setSendServerVersion(false);
			}
		}
		return connector;
	}

	private Handler addHandlerWrappers(Handler handler) {
		if (getCompression() != null && getCompression().getEnabled()) {
			handler = applyWrapper(handler,
					JettyHandlerWrappers.createGzipHandlerWrapper(getCompression()));
		}
		if (StringUtils.hasText(getServerHeader())) {
			handler = applyWrapper(handler, JettyHandlerWrappers
					.createServerHeaderHandlerWrapper(getServerHeader()));
		}
		return handler;
	}

	private Handler applyWrapper(Handler handler, HandlerWrapper wrapper) {
		wrapper.setHandler(handler);
		return wrapper;
	}

	private void customizeSsl(Server server, InetSocketAddress address) {
		new SslServerCustomizer(address, getSsl(), getSslStoreProvider(), getHttp2())
				.customize(server);
	}

	
	public ThreadPool getThreadPool() {
		return this.threadPool;
	}

	
	public void setThreadPool(ThreadPool threadPool) {
		this.threadPool = threadPool;
	}

}
