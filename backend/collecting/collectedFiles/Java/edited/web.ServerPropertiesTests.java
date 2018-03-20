

package org.springframework.boot.autoconfigure.web;

import java.net.InetAddress;
import java.nio.charset.StandardCharsets;
import java.time.Duration;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

import org.junit.Test;

import org.springframework.boot.context.properties.bind.Bindable;
import org.springframework.boot.context.properties.bind.Binder;
import org.springframework.boot.context.properties.source.ConfigurationPropertySource;
import org.springframework.boot.context.properties.source.MapConfigurationPropertySource;

import static org.assertj.core.api.Assertions.assertThat;


public class ServerPropertiesTests {

	private final ServerProperties properties = new ServerProperties();

	@Test
	public void testAddressBinding() throws Exception {
		bind("server.address", "127.0.0.1");
		assertThat(this.properties.getAddress())
				.isEqualTo(InetAddress.getByName("127.0.0.1"));
	}

	@Test
	public void testPortBinding() {
		bind("server.port", "9000");
		assertThat(this.properties.getPort().intValue()).isEqualTo(9000);
	}

	@Test
	public void testServerHeaderDefault() {
		assertThat(this.properties.getServerHeader()).isNull();
	}

	@Test
	public void testServerHeader() {
		bind("server.server-header", "Custom Server");
		assertThat(this.properties.getServerHeader()).isEqualTo("Custom Server");
	}

	@Test
	public void testConnectionTimeout() {
		bind("server.connection-timeout", "60s");
		assertThat(this.properties.getConnectionTimeout())
				.isEqualTo(Duration.ofMillis(60000));
	}

	@Test
	public void testServletPathAsMapping() {
		bind("server.servlet.path", "/foo