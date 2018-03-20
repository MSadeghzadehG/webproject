

package org.springframework.boot.web.server;

import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.util.Arrays;
import java.util.LinkedHashSet;
import java.util.Set;

import org.springframework.util.Assert;


public abstract class AbstractConfigurableWebServerFactory
		implements ConfigurableWebServerFactory {

	private int port = 8080;

	private InetAddress address;

	private Set<ErrorPage> errorPages = new LinkedHashSet<>();

	private Ssl ssl;

	private SslStoreProvider sslStoreProvider;

	private Http2 http2;

	private Compression compression;

	private String serverHeader;

	
	public AbstractConfigurableWebServerFactory() {
	}

	
	public AbstractConfigurableWebServerFactory(int port) {
		this.port = port;
	}

	
	public int getPort() {
		return this.port;
	}

	@Override
	public void setPort(int port) {
		this.port = port;
	}

	
	public InetAddress getAddress() {
		return this.address;
	}

	@Override
	public void setAddress(InetAddress address) {
		this.address = address;
	}

	
	public Set<ErrorPage> getErrorPages() {
		return this.errorPages;
	}

	@Override
	public void setErrorPages(Set<? extends ErrorPage> errorPages) {
		Assert.notNull(errorPages, "ErrorPages must not be null");
		this.errorPages = new LinkedHashSet<>(errorPages);
	}

	@Override
	public void addErrorPages(ErrorPage... errorPages) {
		Assert.notNull(errorPages, "ErrorPages must not be null");
		this.errorPages.addAll(Arrays.asList(errorPages));
	}

	public Ssl getSsl() {
		return this.ssl;
	}

	@Override
	public void setSsl(Ssl ssl) {
		this.ssl = ssl;
	}

	public SslStoreProvider getSslStoreProvider() {
		return this.sslStoreProvider;
	}

	@Override
	public void setSslStoreProvider(SslStoreProvider sslStoreProvider) {
		this.sslStoreProvider = sslStoreProvider;
	}

	public Http2 getHttp2() {
		return this.http2;
	}

	@Override
	public void setHttp2(Http2 http2) {
		this.http2 = http2;
	}

	public Compression getCompression() {
		return this.compression;
	}

	@Override
	public void setCompression(Compression compression) {
		this.compression = compression;
	}

	public String getServerHeader() {
		return this.serverHeader;
	}

	@Override
	public void setServerHeader(String serverHeader) {
		this.serverHeader = serverHeader;
	}

	
	protected final File createTempDir(String prefix) {
		try {
			File tempDir = File.createTempFile(prefix + ".", "." + getPort());
			tempDir.delete();
			tempDir.mkdir();
			tempDir.deleteOnExit();
			return tempDir;
		}
		catch (IOException ex) {
			throw new WebServerException(
					"Unable to create tempDir. java.io.tmpdir is set to "
							+ System.getProperty("java.io.tmpdir"),
					ex);
		}
	}

}
