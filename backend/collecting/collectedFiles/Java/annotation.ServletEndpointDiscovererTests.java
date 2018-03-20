/*
 * Copyright 2012-2018 the original author or authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.springframework.boot.actuate.endpoint.web.annotation;

import java.io.IOException;
import java.util.Collection;
import java.util.Collections;
import java.util.List;
import java.util.function.Consumer;
import java.util.function.Supplier;
import java.util.stream.Collectors;

import javax.servlet.GenericServlet;
import javax.servlet.ServletException;
import javax.servlet.ServletRequest;
import javax.servlet.ServletResponse;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import org.springframework.boot.actuate.endpoint.ExposableEndpoint;
import org.springframework.boot.actuate.endpoint.annotation.DiscoveredEndpoint;
import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.annotation.ReadOperation;
import org.springframework.boot.actuate.endpoint.web.EndpointServlet;
import org.springframework.boot.actuate.endpoint.web.ExposableServletEndpoint;
import org.springframework.boot.actuate.endpoint.web.PathMapper;
import org.springframework.context.annotation.AnnotationConfigApplicationContext;
import org.springframework.context.annotation.Configuration;
import org.springframework.context.annotation.Import;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * Tests for {@link ServletEndpointDiscoverer}.
 *
 * @author Phillip Webb
 */
public class ServletEndpointDiscovererTests {

	@Rule
	public final ExpectedException thrown = ExpectedException.none();

	@Test
	public void getEndpointsWhenNoEndpointBeansShouldReturnEmptyCollection() {
		load(EmptyConfiguration.class,
				(discoverer) -> assertThat(discoverer.getEndpoints()).isEmpty());
	}

	@Test
	public void getEndpointsShouldIncludeServletEndpoints() {
		load(TestServletEndpoint.class, (discoverer) -> {
			Collection<ExposableServletEndpoint> endpoints = discoverer.getEndpoints();
			assertThat(endpoints).hasSize(1);
			ExposableServletEndpoint endpoint = endpoints.iterator().next();
			assertThat(endpoint.getId()).isEqualTo("testservlet");
			assertThat(endpoint.getEndpointServlet()).isNotNull();
			assertThat(endpoint).isInstanceOf(DiscoveredEndpoint.class);
		});
	}

	@Test
	public void getEndpointsShouldNotDiscoverRegularEndpoints() {
		load(WithRegularEndpointConfiguration.class, (discoverer) -> {
			Collection<ExposableServletEndpoint> endpoints = discoverer.getEndpoints();
			List<String> ids = endpoints.stream().map(ExposableEndpoint::getId)
					.collect(Collectors.toList());
			assertThat(ids).containsOnly("testservlet");
		});
	}

	@Test
	public void getEndpointWhenEndpointHasOperationsShouldThrowException() {
		load(TestServletEndpointWithOperation.class, (discoverer) -> {
			this.thrown.expect(IllegalStateException.class);
			this.thrown.expectMessage("ServletEndpoints must not declare operations");
			discoverer.getEndpoints();
		});
	}

	@Test
	public void getEndpointWhenEndpointNotASupplierShouldThrowException() {
		load(TestServletEndpointNotASupplier.class, (discoverer) -> {
			this.thrown.expect(IllegalStateException.class);
			this.thrown.expectMessage("must be a supplier");
			discoverer.getEndpoints();
		});
	}

	@Test
	public void getEndpointWhenEndpointSuppliesWrongTypeShouldThrowException() {
		load(TestServletEndpointSupplierOfWrongType.class, (discoverer) -> {
			this.thrown.expect(IllegalStateException.class);
			this.thrown.expectMessage("must supply an EndpointServlet");
			discoverer.getEndpoints();
		});
	}

	@Test
	public void getEndpointWhenEndpointSuppliesNullShouldThrowException() {
		load(TestServletEndpointSupplierOfNull.class, (discoverer) -> {
			this.thrown.expect(IllegalStateException.class);
			this.thrown.expectMessage("must not supply null");
			discoverer.getEndpoints();
		});
	}

	private void load(Class<?> configuration,
			Consumer<ServletEndpointDiscoverer> consumer) {
		AnnotationConfigApplicationContext context = new AnnotationConfigApplicationContext(
				configuration);
		try {
			ServletEndpointDiscoverer discoverer = new ServletEndpointDiscoverer(context,
					PathMapper.useEndpointId(), Collections.emptyList());
			consumer.accept(discoverer);
		}
		finally {
			context.close();
		}
	}

	@Configuration
	static class EmptyConfiguration {

	}

	@Configuration
	@Import({ TestEndpoint.class, TestServletEndpoint.class })
	static class WithRegularEndpointConfiguration {

	}

	@ServletEndpoint(id = "testservlet")
	static class TestServletEndpoint implements Supplier<EndpointServlet> {

		@Override
		public EndpointServlet get() {
			return new EndpointServlet(TestServlet.class);
		}

	}

	@Endpoint(id = "test")
	static class TestEndpoint {

	}

	@ServletEndpoint(id = "testservlet")
	static class TestServletEndpointWithOperation implements Supplier<EndpointServlet> {

		@Override
		public EndpointServlet get() {
			return new EndpointServlet(TestServlet.class);
		}

		@ReadOperation
		public String read() {
			return "error";
		}

	}

	private static class TestServlet extends GenericServlet {

		@Override
		public void service(ServletRequest req, ServletResponse res)
				throws ServletException, IOException {
		}

	}

	@ServletEndpoint(id = "testservlet")
	static class TestServletEndpointNotASupplier {

	}

	@ServletEndpoint(id = "testservlet")
	static class TestServletEndpointSupplierOfWrongType implements Supplier<String> {

		@Override
		public String get() {
			return "error";
		}

	}

	@ServletEndpoint(id = "testservlet")
	static class TestServletEndpointSupplierOfNull implements Supplier<EndpointServlet> {

		@Override
		public EndpointServlet get() {
			return null;
		}

	}

}
