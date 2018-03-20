

package org.springframework.boot.actuate.autoconfigure.security.reactive;

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

import reactor.core.publisher.Mono;

import org.springframework.beans.factory.NoSuchBeanDefinitionException;
import org.springframework.boot.actuate.autoconfigure.endpoint.web.WebEndpointProperties;
import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.web.PathMappedEndpoints;
import org.springframework.boot.security.reactive.ApplicationContextServerWebExchangeMatcher;
import org.springframework.core.annotation.AnnotationUtils;
import org.springframework.security.web.server.util.matcher.OrServerWebExchangeMatcher;
import org.springframework.security.web.server.util.matcher.PathPatternParserServerWebExchangeMatcher;
import org.springframework.security.web.server.util.matcher.ServerWebExchangeMatcher;
import org.springframework.security.web.server.util.matcher.ServerWebExchangeMatcher.MatchResult;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;
import org.springframework.web.server.ServerWebExchange;


public final class EndpointRequest {

	private static final ServerWebExchangeMatcher EMPTY_MATCHER = (request) -> MatchResult
			.notMatch();

	private EndpointRequest() {
	}

	
	public static EndpointServerWebExchangeMatcher toAnyEndpoint() {
		return new EndpointServerWebExchangeMatcher(true);
	}

	
	public static EndpointServerWebExchangeMatcher to(Class<?>... endpoints) {
		return new EndpointServerWebExchangeMatcher(endpoints, false);
	}

	
	public static EndpointServerWebExchangeMatcher to(String... endpoints) {
		return new EndpointServerWebExchangeMatcher(endpoints, false);
	}

	
	public static LinksServerWebExchangeMatcher toLinks() {
		return new LinksServerWebExchangeMatcher();
	}

	
	public static final class EndpointServerWebExchangeMatcher
			extends ApplicationContextServerWebExchangeMatcher<PathMappedEndpoints> {

		private final List<Object> includes;

		private final List<Object> excludes;

		private final boolean includeLinks;

		private ServerWebExchangeMatcher delegate;

		private EndpointServerWebExchangeMatcher(boolean includeLinks) {
			this(Collections.emptyList(), Collections.emptyList(), includeLinks);
		}

		private EndpointServerWebExchangeMatcher(Class<?>[] endpoints,
				boolean includeLinks) {
			this(Arrays.asList((Object[]) endpoints), Collections.emptyList(),
					includeLinks);
		}

		private EndpointServerWebExchangeMatcher(String[] endpoints,
				boolean includeLinks) {
			this(Arrays.asList((Object[]) endpoints), Collections.emptyList(),
					includeLinks);
		}

		private EndpointServerWebExchangeMatcher(List<Object> includes,
				List<Object> excludes, boolean includeLinks) {
			super(PathMappedEndpoints.class);
			this.includes = includes;
			this.excludes = excludes;
			this.includeLinks = includeLinks;
		}

		public EndpointServerWebExchangeMatcher excluding(Class<?>... endpoints) {
			List<Object> excludes = new ArrayList<>(this.excludes);
			excludes.addAll(Arrays.asList((Object[]) endpoints));
			return new EndpointServerWebExchangeMatcher(this.includes, excludes,
					this.includeLinks);
		}

		public EndpointServerWebExchangeMatcher excluding(String... endpoints) {
			List<Object> excludes = new ArrayList<>(this.excludes);
			excludes.addAll(Arrays.asList((Object[]) endpoints));
			return new EndpointServerWebExchangeMatcher(this.includes, excludes,
					this.includeLinks);
		}

		public EndpointServerWebExchangeMatcher excludingLinks() {
			return new EndpointServerWebExchangeMatcher(this.includes, this.excludes,
					false);
		}

		@Override
		protected void initialized(Supplier<PathMappedEndpoints> pathMappedEndpoints) {
			this.delegate = createDelegate(pathMappedEndpoints);
		}

		private ServerWebExchangeMatcher createDelegate(
				Supplier<PathMappedEndpoints> pathMappedEndpoints) {
			try {
				return createDelegate(pathMappedEndpoints.get());
			}
			catch (NoSuchBeanDefinitionException ex) {
				return EMPTY_MATCHER;
			}
		}

		private ServerWebExchangeMatcher createDelegate(
				PathMappedEndpoints pathMappedEndpoints) {
			Set<String> paths = new LinkedHashSet<>();
			if (this.includes.isEmpty()) {
				paths.addAll(pathMappedEndpoints.getAllPaths());
			}
			streamPaths(this.includes, pathMappedEndpoints).forEach(paths::add);
			streamPaths(this.excludes, pathMappedEndpoints).forEach(paths::remove);
			List<ServerWebExchangeMatcher> delegateMatchers = getDelegateMatchers(paths);
			if (this.includeLinks
					&& StringUtils.hasText(pathMappedEndpoints.getBasePath())) {
				delegateMatchers.add(new PathPatternParserServerWebExchangeMatcher(
						pathMappedEndpoints.getBasePath()));
			}
			return new OrServerWebExchangeMatcher(delegateMatchers);
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

		private List<ServerWebExchangeMatcher> getDelegateMatchers(Set<String> paths) {
			return paths.stream().map(
					(path) -> new PathPatternParserServerWebExchangeMatcher(path + "
	public static final class LinksServerWebExchangeMatcher
			extends ApplicationContextServerWebExchangeMatcher<WebEndpointProperties> {

		private ServerWebExchangeMatcher delegate;

		private LinksServerWebExchangeMatcher() {
			super(WebEndpointProperties.class);
		}

		@Override
		protected void initialized(Supplier<WebEndpointProperties> properties) {
			this.delegate = createDelegate(properties.get());
		}

		private ServerWebExchangeMatcher createDelegate(
				WebEndpointProperties properties) {
			if (StringUtils.hasText(properties.getBasePath())) {
				return new PathPatternParserServerWebExchangeMatcher(
						properties.getBasePath());
			}
			return EMPTY_MATCHER;
		}

		@Override
		protected Mono<MatchResult> matches(ServerWebExchange exchange,
				Supplier<WebEndpointProperties> context) {
			return this.delegate.matches(exchange);
		}

	}

}
