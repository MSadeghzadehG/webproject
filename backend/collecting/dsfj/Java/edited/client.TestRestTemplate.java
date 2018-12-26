

package org.springframework.boot.test.web.client;

import java.io.IOException;
import java.lang.reflect.Field;
import java.net.URI;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.http.client.HttpClient;
import org.apache.http.client.config.CookieSpecs;
import org.apache.http.client.config.RequestConfig;
import org.apache.http.client.config.RequestConfig.Builder;
import org.apache.http.client.protocol.HttpClientContext;
import org.apache.http.conn.ssl.SSLConnectionSocketFactory;
import org.apache.http.conn.ssl.TrustSelfSignedStrategy;
import org.apache.http.impl.client.HttpClients;
import org.apache.http.protocol.HttpContext;
import org.apache.http.ssl.SSLContextBuilder;

import org.springframework.boot.web.client.RestTemplateBuilder;
import org.springframework.boot.web.client.RootUriTemplateHandler;
import org.springframework.core.ParameterizedTypeReference;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.RequestEntity;
import org.springframework.http.ResponseEntity;
import org.springframework.http.client.ClientHttpRequestFactory;
import org.springframework.http.client.ClientHttpRequestInterceptor;
import org.springframework.http.client.ClientHttpResponse;
import org.springframework.http.client.HttpComponentsClientHttpRequestFactory;
import org.springframework.http.client.InterceptingClientHttpRequestFactory;
import org.springframework.http.client.support.BasicAuthorizationInterceptor;
import org.springframework.util.Assert;
import org.springframework.util.ReflectionUtils;
import org.springframework.web.client.DefaultResponseErrorHandler;
import org.springframework.web.client.RequestCallback;
import org.springframework.web.client.ResponseExtractor;
import org.springframework.web.client.RestClientException;
import org.springframework.web.client.RestTemplate;
import org.springframework.web.util.DefaultUriBuilderFactory;
import org.springframework.web.util.UriTemplateHandler;


public class TestRestTemplate {

	private final RestTemplate restTemplate;

	private final HttpClientOption[] httpClientOptions;

	
	public TestRestTemplate(RestTemplateBuilder restTemplateBuilder) {
		this(restTemplateBuilder, null, null);
	}

	
	public TestRestTemplate(HttpClientOption... httpClientOptions) {
		this(null, null, httpClientOptions);
	}

	
	public TestRestTemplate(String username, String password,
			HttpClientOption... httpClientOptions) {
		this(new RestTemplateBuilder(), username, password, httpClientOptions);
	}

	
	public TestRestTemplate(RestTemplateBuilder restTemplateBuilder, String username,
			String password, HttpClientOption... httpClientOptions) {
		this(restTemplateBuilder == null ? null : restTemplateBuilder.build(), username,
				password, httpClientOptions);
	}

	private TestRestTemplate(RestTemplate restTemplate, String username, String password,
			HttpClientOption... httpClientOptions) {
		Assert.notNull(restTemplate, "RestTemplate must not be null");
		this.httpClientOptions = httpClientOptions;
		if (getRequestFactoryClass(restTemplate).isAssignableFrom(
				HttpComponentsClientHttpRequestFactory.class)) {
			restTemplate.setRequestFactory(
					new CustomHttpComponentsClientHttpRequestFactory(httpClientOptions));
		}
		addAuthentication(restTemplate, username, password);
		restTemplate.setErrorHandler(new NoOpResponseErrorHandler());
		this.restTemplate = restTemplate;
	}

	private Class<? extends ClientHttpRequestFactory> getRequestFactoryClass(
			RestTemplate restTemplate) {
		ClientHttpRequestFactory requestFactory = restTemplate.getRequestFactory();
		if (InterceptingClientHttpRequestFactory.class
				.isAssignableFrom(requestFactory.getClass())) {
			Field requestFactoryField = ReflectionUtils.findField(RestTemplate.class,
					"requestFactory");
			ReflectionUtils.makeAccessible(requestFactoryField);
			requestFactory = (ClientHttpRequestFactory) ReflectionUtils
					.getField(requestFactoryField, restTemplate);
		}
		return requestFactory.getClass();
	}

	private void addAuthentication(RestTemplate restTemplate, String username,
			String password) {
		if (username == null) {
			return;
		}
		List<ClientHttpRequestInterceptor> interceptors = restTemplate.getInterceptors();
		if (interceptors == null) {
			interceptors = Collections.emptyList();
		}
		interceptors = new ArrayList<>(interceptors);
		interceptors.removeIf(BasicAuthorizationInterceptor.class::isInstance);
		interceptors.add(new BasicAuthorizationInterceptor(username, password));
		restTemplate.setInterceptors(interceptors);
	}

	
	public void setUriTemplateHandler(UriTemplateHandler handler) {
		this.restTemplate.setUriTemplateHandler(handler);
	}

	
	public String getRootUri() {
		UriTemplateHandler uriTemplateHandler = this.restTemplate.getUriTemplateHandler();
		if (uriTemplateHandler instanceof RootUriTemplateHandler) {
			return ((RootUriTemplateHandler) uriTemplateHandler).getRootUri();
		}
		return "";
	}

	
	public <T> T getForObject(String url, Class<T> responseType, Object... urlVariables)
			throws RestClientException {
		return this.restTemplate.getForObject(url, responseType, urlVariables);
	}

	
	public <T> T getForObject(String url, Class<T> responseType,
			Map<String, ?> urlVariables) throws RestClientException {
		return this.restTemplate.getForObject(url, responseType, urlVariables);
	}

	
	public <T> T getForObject(URI url, Class<T> responseType) throws RestClientException {
		return this.restTemplate.getForObject(applyRootUriIfNecessary(url), responseType);
	}

	
	public <T> ResponseEntity<T> getForEntity(String url, Class<T> responseType,
			Object... urlVariables) throws RestClientException {
		return this.restTemplate.getForEntity(url, responseType, urlVariables);
	}

	
	public <T> ResponseEntity<T> getForEntity(String url, Class<T> responseType,
			Map<String, ?> urlVariables) throws RestClientException {
		return this.restTemplate.getForEntity(url, responseType, urlVariables);
	}

	
	public <T> ResponseEntity<T> getForEntity(URI url, Class<T> responseType)
			throws RestClientException {
		return this.restTemplate.getForEntity(applyRootUriIfNecessary(url), responseType);
	}

	
	public HttpHeaders headForHeaders(String url, Object... urlVariables)
			throws RestClientException {
		return this.restTemplate.headForHeaders(url, urlVariables);
	}

	
	public HttpHeaders headForHeaders(String url, Map<String, ?> urlVariables)
			throws RestClientException {
		return this.restTemplate.headForHeaders(url, urlVariables);
	}

	
	public HttpHeaders headForHeaders(URI url) throws RestClientException {
		return this.restTemplate.headForHeaders(applyRootUriIfNecessary(url));
	}

	
	public URI postForLocation(String url, Object request, Object... urlVariables)
			throws RestClientException {
		return this.restTemplate.postForLocation(url, request, urlVariables);
	}

	
	public URI postForLocation(String url, Object request, Map<String, ?> urlVariables)
			throws RestClientException {
		return this.restTemplate.postForLocation(url, request, urlVariables);
	}

	
	public URI postForLocation(URI url, Object request) throws RestClientException {
		return this.restTemplate.postForLocation(applyRootUriIfNecessary(url), request);
	}

	
	public <T> T postForObject(String url, Object request, Class<T> responseType,
			Object... urlVariables) throws RestClientException {
		return this.restTemplate.postForObject(url, request, responseType, urlVariables);
	}

	
	public <T> T postForObject(String url, Object request, Class<T> responseType,
			Map<String, ?> urlVariables) throws RestClientException {
		return this.restTemplate.postForObject(url, request, responseType, urlVariables);
	}

	
	public <T> T postForObject(URI url, Object request, Class<T> responseType)
			throws RestClientException {
		return this.restTemplate.postForObject(applyRootUriIfNecessary(url), request,
				responseType);
	}

	
	public <T> ResponseEntity<T> postForEntity(String url, Object request,
			Class<T> responseType, Object... urlVariables) throws RestClientException {
		return this.restTemplate.postForEntity(url, request, responseType, urlVariables);
	}

	
	public <T> ResponseEntity<T> postForEntity(String url, Object request,
			Class<T> responseType, Map<String, ?> urlVariables)
			throws RestClientException {
		return this.restTemplate.postForEntity(url, request, responseType, urlVariables);
	}

	
	public <T> ResponseEntity<T> postForEntity(URI url, Object request,
			Class<T> responseType) throws RestClientException {
		return this.restTemplate.postForEntity(applyRootUriIfNecessary(url), request,
				responseType);
	}

	
	public void put(String url, Object request, Object... urlVariables)
			throws RestClientException {
		this.restTemplate.put(url, request, urlVariables);
	}

	
	public void put(String url, Object request, Map<String, ?> urlVariables)
			throws RestClientException {
		this.restTemplate.put(url, request, urlVariables);
	}

	
	public void put(URI url, Object request) throws RestClientException {
		this.restTemplate.put(applyRootUriIfNecessary(url), request);
	}

	
	public <T> T patchForObject(String url, Object request, Class<T> responseType,
			Object... uriVariables) throws RestClientException {
		return this.restTemplate.patchForObject(url, request, responseType, uriVariables);
	}

	
	public <T> T patchForObject(String url, Object request, Class<T> responseType,
			Map<String, ?> uriVariables) throws RestClientException {
		return this.restTemplate.patchForObject(url, request, responseType, uriVariables);
	}

	
	public <T> T patchForObject(URI url, Object request, Class<T> responseType)
			throws RestClientException {
		return this.restTemplate.patchForObject(applyRootUriIfNecessary(url), request,
				responseType);

	}

	
	public void delete(String url, Object... urlVariables) throws RestClientException {
		this.restTemplate.delete(url, urlVariables);
	}

	
	public void delete(String url, Map<String, ?> urlVariables)
			throws RestClientException {
		this.restTemplate.delete(url, urlVariables);
	}

	
	public void delete(URI url) throws RestClientException {
		this.restTemplate.delete(applyRootUriIfNecessary(url));
	}

	
	public Set<HttpMethod> optionsForAllow(String url, Object... urlVariables)
			throws RestClientException {
		return this.restTemplate.optionsForAllow(url, urlVariables);
	}

	
	public Set<HttpMethod> optionsForAllow(String url, Map<String, ?> urlVariables)
			throws RestClientException {
		return this.restTemplate.optionsForAllow(url, urlVariables);
	}

	
	public Set<HttpMethod> optionsForAllow(URI url) throws RestClientException {
		return this.restTemplate.optionsForAllow(applyRootUriIfNecessary(url));
	}

	
	public <T> ResponseEntity<T> exchange(String url, HttpMethod method,
			HttpEntity<?> requestEntity, Class<T> responseType, Object... urlVariables)
			throws RestClientException {
		return this.restTemplate.exchange(url, method, requestEntity, responseType,
				urlVariables);
	}

	
	public <T> ResponseEntity<T> exchange(String url, HttpMethod method,
			HttpEntity<?> requestEntity, Class<T> responseType,
			Map<String, ?> urlVariables) throws RestClientException {
		return this.restTemplate.exchange(url, method, requestEntity, responseType,
				urlVariables);
	}

	
	public <T> ResponseEntity<T> exchange(URI url, HttpMethod method,
			HttpEntity<?> requestEntity, Class<T> responseType)
			throws RestClientException {
		return this.restTemplate.exchange(applyRootUriIfNecessary(url), method,
				requestEntity, responseType);
	}

	
	public <T> ResponseEntity<T> exchange(String url, HttpMethod method,
			HttpEntity<?> requestEntity, ParameterizedTypeReference<T> responseType,
			Object... urlVariables) throws RestClientException {
		return this.restTemplate.exchange(url, method, requestEntity, responseType,
				urlVariables);
	}

	
	public <T> ResponseEntity<T> exchange(String url, HttpMethod method,
			HttpEntity<?> requestEntity, ParameterizedTypeReference<T> responseType,
			Map<String, ?> urlVariables) throws RestClientException {
		return this.restTemplate.exchange(url, method, requestEntity, responseType,
				urlVariables);
	}

	
	public <T> ResponseEntity<T> exchange(URI url, HttpMethod method,
			HttpEntity<?> requestEntity, ParameterizedTypeReference<T> responseType)
			throws RestClientException {
		return this.restTemplate.exchange(applyRootUriIfNecessary(url), method,
				requestEntity, responseType);
	}

	
	public <T> ResponseEntity<T> exchange(RequestEntity<?> requestEntity,
			Class<T> responseType) throws RestClientException {
		return this.restTemplate.exchange(
				createRequestEntityWithRootAppliedUri(requestEntity), responseType);
	}

	
	public <T> ResponseEntity<T> exchange(RequestEntity<?> requestEntity,
			ParameterizedTypeReference<T> responseType) throws RestClientException {
		return this.restTemplate.exchange(
				createRequestEntityWithRootAppliedUri(requestEntity), responseType);
	}

	
	public <T> T execute(String url, HttpMethod method, RequestCallback requestCallback,
			ResponseExtractor<T> responseExtractor, Object... urlVariables)
			throws RestClientException {
		return this.restTemplate.execute(url, method, requestCallback, responseExtractor,
				urlVariables);
	}

	
	public <T> T execute(String url, HttpMethod method, RequestCallback requestCallback,
			ResponseExtractor<T> responseExtractor, Map<String, ?> urlVariables)
			throws RestClientException {
		return this.restTemplate.execute(url, method, requestCallback, responseExtractor,
				urlVariables);
	}

	
	public <T> T execute(URI url, HttpMethod method, RequestCallback requestCallback,
			ResponseExtractor<T> responseExtractor) throws RestClientException {
		return this.restTemplate.execute(applyRootUriIfNecessary(url), method,
				requestCallback, responseExtractor);
	}

	
	public RestTemplate getRestTemplate() {
		return this.restTemplate;
	}

	
	public TestRestTemplate withBasicAuth(String username, String password) {
		RestTemplate restTemplate = new RestTemplateBuilder()
				.messageConverters(getRestTemplate().getMessageConverters())
				.interceptors(getRestTemplate().getInterceptors())
				.uriTemplateHandler(getRestTemplate().getUriTemplateHandler()).build();
		TestRestTemplate testRestTemplate = new TestRestTemplate(restTemplate, username,
				password, this.httpClientOptions);
		testRestTemplate.getRestTemplate()
				.setErrorHandler(getRestTemplate().getErrorHandler());
		return testRestTemplate;
	}

	@SuppressWarnings({ "rawtypes", "unchecked" })
	private RequestEntity<?> createRequestEntityWithRootAppliedUri(
			RequestEntity<?> requestEntity) {
		return new RequestEntity(requestEntity.getBody(), requestEntity.getHeaders(),
				requestEntity.getMethod(),
				applyRootUriIfNecessary(requestEntity.getUrl()), requestEntity.getType());
	}

	private URI applyRootUriIfNecessary(URI uri) {
		UriTemplateHandler uriTemplateHandler = this.restTemplate.getUriTemplateHandler();
		if ((uriTemplateHandler instanceof RootUriTemplateHandler)
				&& uri.toString().startsWith("/")) {
			return URI.create(((RootUriTemplateHandler) uriTemplateHandler).getRootUri()
					+ uri.toString());
		}
		return uri;
	}

	
	public enum HttpClientOption {

		
		ENABLE_COOKIES,

		
		ENABLE_REDIRECTS,

		
		SSL

	}

	
	protected static class CustomHttpComponentsClientHttpRequestFactory
			extends HttpComponentsClientHttpRequestFactory {

		private final String cookieSpec;

		private final boolean enableRedirects;

		public CustomHttpComponentsClientHttpRequestFactory(
				HttpClientOption[] httpClientOptions) {
			Set<HttpClientOption> options = new HashSet<>(
					Arrays.asList(httpClientOptions));
			this.cookieSpec = (options.contains(HttpClientOption.ENABLE_COOKIES)
					? CookieSpecs.STANDARD : CookieSpecs.IGNORE_COOKIES);
			this.enableRedirects = options.contains(HttpClientOption.ENABLE_REDIRECTS);
			if (options.contains(HttpClientOption.SSL)) {
				setHttpClient(createSslHttpClient());
			}
		}

		private HttpClient createSslHttpClient() {
			try {
				SSLConnectionSocketFactory socketFactory = new SSLConnectionSocketFactory(
						new SSLContextBuilder()
								.loadTrustMaterial(null, new TrustSelfSignedStrategy())
								.build());
				return HttpClients.custom().setSSLSocketFactory(socketFactory).build();
			}
			catch (Exception ex) {
				throw new IllegalStateException("Unable to create SSL HttpClient", ex);
			}
		}

		@Override
		protected HttpContext createHttpContext(HttpMethod httpMethod, URI uri) {
			HttpClientContext context = HttpClientContext.create();
			context.setRequestConfig(getRequestConfig());
			return context;
		}

		protected RequestConfig getRequestConfig() {
			Builder builder = RequestConfig.custom().setCookieSpec(this.cookieSpec)
					.setAuthenticationEnabled(false)
					.setRedirectsEnabled(this.enableRedirects);
			return builder.build();
		}

	}

	private static class NoOpResponseErrorHandler extends DefaultResponseErrorHandler {

		@Override
		public void handleError(ClientHttpResponse response) throws IOException {
		}

	}

}
