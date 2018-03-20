

package org.springframework.boot.actuate.endpoint.web.test;

import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;

import javax.ws.rs.core.MediaType;

import org.glassfish.jersey.server.ResourceConfig;
import org.glassfish.jersey.server.model.Resource;
import org.junit.runners.BlockJUnit4ClassRunner;
import org.junit.runners.model.InitializationError;

import org.springframework.boot.actuate.endpoint.http.ActuatorMediaType;
import org.springframework.boot.actuate.endpoint.invoke.convert.ConversionServiceParameterValueMapper;
import org.springframework.boot.actuate.endpoint.web.EndpointLinksResolver;
import org.springframework.boot.actuate.endpoint.web.EndpointMapping;
import org.springframework.boot.actuate.endpoint.web.EndpointMediaTypes;
import org.springframework.boot.actuate.endpoint.web.PathMapper;
import org.springframework.boot.actuate.endpoint.web.annotation.WebEndpointDiscoverer;
import org.springframework.boot.actuate.endpoint.web.jersey.JerseyEndpointResourceFactory;
import org.springframework.boot.autoconfigure.ImportAutoConfiguration;
import org.springframework.boot.autoconfigure.jackson.JacksonAutoConfiguration;
import org.springframework.boot.autoconfigure.jersey.JerseyAutoConfiguration;
import org.springframework.boot.autoconfigure.jersey.ResourceConfigCustomizer;
import org.springframework.boot.web.embedded.tomcat.TomcatServletWebServerFactory;
import org.springframework.boot.web.servlet.context.AnnotationConfigServletWebServerApplicationContext;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ConfigurableApplicationContext;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.util.ClassUtils;


class JerseyEndpointsRunner extends AbstractWebEndpointRunner {

	JerseyEndpointsRunner(Class<?> testClass) throws InitializationError {
		super(testClass, "Jersey", JerseyEndpointsRunner::createContext);
	}

	private static ConfigurableApplicationContext createContext(List<Class<?>> classes) {
		AnnotationConfigServletWebServerApplicationContext context = new AnnotationConfigServletWebServerApplicationContext();
		classes.add(JerseyEndpointConfiguration.class);
		context.register(ClassUtils.toClassArray(classes));
		context.refresh();
		return context;
	}

	@Configuration
	@ImportAutoConfiguration({ JacksonAutoConfiguration.class,
			JerseyAutoConfiguration.class })
	static class JerseyEndpointConfiguration {

		private final ApplicationContext applicationContext;

		JerseyEndpointConfiguration(ApplicationContext applicationContext) {
			this.applicationContext = applicationContext;
		}

		@Bean
		public TomcatServletWebServerFactory tomcat() {
			return new TomcatServletWebServerFactory(0);
		}

		@Bean
		public ResourceConfig resourceConfig() {
			return new ResourceConfig();
		}

		@Bean
		public ResourceConfigCustomizer webEndpointRegistrar() {
			return this::customize;
		}

		private void customize(ResourceConfig config) {
			List<String> mediaTypes = Arrays.asList(MediaType.APPLICATION_JSON,
					ActuatorMediaType.V2_JSON);
			EndpointMediaTypes endpointMediaTypes = new EndpointMediaTypes(mediaTypes,
					mediaTypes);
			WebEndpointDiscoverer discoverer = new WebEndpointDiscoverer(
					this.applicationContext, new ConversionServiceParameterValueMapper(),
					endpointMediaTypes, PathMapper.useEndpointId(),
					Collections.emptyList(), Collections.emptyList());
			Collection<Resource> resources = new JerseyEndpointResourceFactory()
					.createEndpointResources(new EndpointMapping("/actuator"),
							discoverer.getEndpoints(), endpointMediaTypes,
							new EndpointLinksResolver(discoverer.getEndpoints()));
			config.registerResources(new HashSet<>(resources));
		}

	}

}