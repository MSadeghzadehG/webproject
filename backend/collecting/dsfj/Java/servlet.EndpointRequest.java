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

package org.springframework.boot.actuate.autoconfigure.security.servlet;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.function.Supplier;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import javax.servlet.http.HttpServletRequest;

import org.springframework.beans.factory.NoSuchBeanDefinitionException;
import org.springframework.boot.actuate.autoconfigure.endpoint.web.WebEndpointProperties;
import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.web.PathMappedEndpoints;
import org.springframework.boot.security.servlet.ApplicationContextRequestMatcher;
import org.springframework.core.annotation.AnnotationUtils;
import org.springframework.security.web.util.matcher.AntPathRequestMatcher;
import org.springframework.security.web.util.matcher.OrRequestMatcher;
import org.springframework.security.web.util.matcher.RequestMatcher;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;

/**
 * Factory that can be used to create a {@link RequestMatcher} for actuator endpoint
 * locations.
 *
 * @author Madhura Bhave
 * @author Phillip Webb
 * @since 2.0.0
 */
public final class EndpointRequest {

	private static final RequestMatcher EMPTY_MATCHER = (request) -> false;

	private EndpointRequest() {
	}

	/**
	 * Returns a matcher that includes all {@link Endpoint actuator endpoints}. It also
	 * includes the links endpoint which is present at the base path of the actuator
	 * endpoints. The {@link EndpointRequestMatcher#excluding(Class...) excluding} method
	 * can be used to further remove specific endpoints if required. For example:
	 * <pre class="code">
	 * EndpointRequest.toAnyEndpoint().excluding(ShutdownEndpoint.class)
	 * </pre>
	 * @return the configured {@link RequestMatcher}
	 */
	public static EndpointRequestMatcher toAnyEndpoint() {
		return new EndpointRequestMatcher(true);
	}

	/**
	 * Returns a matcher that includes the specified {@link Endpoint actuator endpoints}.
	 * For example: <pre class="code">
	 * EndpointRequest.to(ShutdownEndpoint.class, HealthEndpoint.class)
	 * </pre>
	 * @param endpoints the endpoints to include
	 * @return the configured {@link RequestMatcher}
	 */
	public static EndpointRequestMatcher to(Class<?>... endpoints) {
		return new EndpointRequestMatcher(endpoints, false);
	}

	/**
	 * Returns a matcher that includes the specified {@link Endpoint actuator endpoints}.
	 * For example: <pre class="code">
	 * EndpointRequest.to("shutdown", "health")
	 * </pre>
	 * @param endpoints the endpoints to include
	 * @return the configured {@link RequestMatcher}
	 */
	public static EndpointRequestMatcher to(String... endpoints) {
		return new EndpointRequestMatcher(endpoints, false);
	}

	/**
	 * Returns a matcher that matches only on the links endpoint. It can be used when
	 * security configuration for the links endpoint is different from the other
	 * {@link Endpoint actuator endpoints}. The
	 * {@link EndpointRequestMatcher#excludingLinks() excludingLinks} method can be used
	 * in combination with this to remove the links endpoint from
	 * {@link EndpointRequest#toAnyEndpoint() toAnyEndpoint}. For example:
	 * <pre class="code">
	 * EndpointRequest.toLinks()
	 * </pre>
	 * @return the configured {@link RequestMatcher}
	 */
	public static LinksRequestMatcher toLinks() {
		return new LinksRequestMatcher();
	}

	/**
	 * The request matcher used to match against {@link Endpoint actuator endpoints}.
	 */
	public static final class EndpointRequestMatcher
			extends ApplicationContextRequestMatcher<PathMappedEndpoints> {

		private final List<Object> includes;

		private final List<Object> excludes;

		private final boolean includeLinks;

		private volatile RequestMatcher delegate;

		private EndpointRequestMatcher(boolean includeLinks) {
			this(Collections.emptyList(), Collections.emptyList(), includeLinks);
		}

		private EndpointRequestMatcher(Class<?>[] endpoints, boolean includeLinks) {
			this(Arrays.asList((Object[]) endpoints), Collections.emptyList(),
					includeLinks);
		}

		private EndpointRequestMatcher(String[] endpoints, boolean includeLinks) {
			this(Arrays.asList((Object[]) endpoints), Collections.emptyList(),
					includeLinks);
		}

		private EndpointRequestMatcher(List<Object> includes, List<Object> excludes,
				boolean includeLinks) {
			super(PathMappedEndpoints.class);
			this.includes = includes;
			this.excludes = excludes;
			this.includeLinks = includeLinks;
		}

		public EndpointRequestMatcher excluding(Class<?>... endpoints) {
			List<Object> excludes = new ArrayList<>(this.excludes);
			excludes.addAll(Arrays.asList((Object[]) endpoints));
			return new EndpointRequestMatcher(this.includes, excludes, this.includeLinks);
		}

		public EndpointRequestMatcher excluding(String... endpoints) {
			List<Object> excludes = new ArrayList<>(this.excludes);
			excludes.addAll(Arrays.asList((Object[]) endpoints));
			return new EndpointRequestMatcher(this.includes, excludes, this.includeLinks);
		}

		public EndpointRequestMatcher excludingLinks() {
			return new EndpointRequestMatcher(this.includes, this.excludes, false);
		}

		@Override
		protected void initialized(Supplier<PathMappedEndpoints> pathMappedEndpoints) {
			this.delegate = createDelegate(pathMappedEndpoints);
		}

		private RequestMatcher createDelegate(
				Supplier<PathMappedEndpoints> pathMappedEndpoints) {
			try {
				return createDelegate(pathMappedEndpoints.get());
			}
			catch (NoSuchBeanDefinitionException ex) {
				return EMPTY_MATCHER;
			}
		}

		private RequestMatcher createDelegate(PathMappedEndpoints pathMappedEndpoints) {
			Set<String> paths = new LinkedHashSet<>();
			if (this.includes.isEmpty()) {
				paths.addAll(pathMappedEndpoints.getAllPaths());
			}
			streamPaths(this.includes, pathMappedEndpoints).forEach(paths::add);
			streamPaths(this.excludes, pathMappedEndpoints).forEach(paths::remove);
			List<RequestMatcher> delegateMatchers = getDelegateMatchers(paths);
			if (this.includeLinks
					&& StringUtils.hasText(pathMappedEndpoints.getBasePath())) {
				delegateMatchers.add(
						new AntPathRequestMatcher(pathMappedEndpoints.getBasePath()));
			}
			return new OrRequestMatcher(delegateMatchers);
		}

		private Stream<String> streamPaths(List<Object> source,
				PathMappedEndpoints pathMappedEndpoints) {
			return source.stream().filter(Objects::nonNull).map(this::getEndpointId)
					.map(pathMappedEndpoints::getPath);
		}

		private String getEndpointId(Object source) {
			if (source instanceof String) {
				return (String) source;
			}
			if (source instanceof Class) {
				return getEndpointId((Class<?>) source);
			}
			throw new IllegalStateException("Unsupported source " + source);
		}

		private String getEndpointId(Class<?> source) {
			Endpoint annotation = AnnotationUtils.findAnnotation(source, Endpoint.class);
			Assert.state(annotation != null,
					() -> "Class " + source + " is not annotated with @Endpoint");
			return annotation.id();
		}

		private List<RequestMatcher> getDelegateMatchers(Set<String> paths) {
			return paths.stream().map((path) -> new AntPathRequestMatcher(path + "/**"))
					.collect(Collectors.toList());
		}

		@Override
		protected boolean matches(HttpServletRequest request,
				Supplier<PathMappedEndpoints> context) {
			return this.delegate.matches(request);
		}

	}

	/**
	 * The request matcher used to match against the links endpoint.
	 */
	public static final class LinksRequestMatcher
			extends ApplicationContextRequestMatcher<WebEndpointProperties> {

		private RequestMatcher delegate;

		private LinksRequestMatcher() {
			super(WebEndpointProperties.class);
		}

		@Override
		protected void initialized(Supplier<WebEndpointProperties> properties) {
			this.delegate = createDelegate(properties.get());
		}

		private RequestMatcher createDelegate(WebEndpointProperties properties) {
			if (StringUtils.hasText(properties.getBasePath())) {
				return new AntPathRequestMatcher(properties.getBasePath());
			}
			return EMPTY_MATCHER;
		}

		@Override
		protected boolean matches(HttpServletRequest request,
				Supplier<WebEndpointProperties> context) {
			return this.delegate.matches(request);
		}

	}

}
