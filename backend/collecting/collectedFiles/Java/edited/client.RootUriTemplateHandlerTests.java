

package org.springframework.boot.web.client;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.HashMap;
import java.util.Map;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import org.springframework.web.client.RestTemplate;
import org.springframework.web.util.UriTemplateHandler;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.verify;


public class RootUriTemplateHandlerTests {

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	private URI uri;

	@Mock
	public UriTemplateHandler delegate;

	public UriTemplateHandler handler;

	@Before
	@SuppressWarnings("unchecked")
	public void setup() throws URISyntaxException {
		MockitoAnnotations.initMocks(this);
		this.uri = new URI("http:		this.handler = new RootUriTemplateHandler("http:		given(this.delegate.expand(anyString(), any(Map.class))).willReturn(this.uri);
		given(this.delegate.expand(anyString(), any(Object[].class)))
				.willReturn(this.uri);
	}

	@Test
	public void createWithNullRootUriShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("RootUri must not be null");
		new RootUriTemplateHandler((String) null);
	}

	@Test
	public void createWithNullHandlerShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("Handler must not be null");
		new RootUriTemplateHandler("http:	}

	@Test
	public void expandMapVariablesShouldPrefixRoot() {
		HashMap<String, Object> uriVariables = new HashMap<>();
		URI expanded = this.handler.expand("/hello", uriVariables);
		verify(this.delegate).expand("http:		assertThat(expanded).isEqualTo(this.uri);
	}

	@Test
	public void expandMapVariablesWhenPathDoesNotStartWithSlashShouldNotPrefixRoot() {
		HashMap<String, Object> uriVariables = new HashMap<>();
		URI expanded = this.handler.expand("http:		verify(this.delegate).expand("http:		assertThat(expanded).isEqualTo(this.uri);
	}

	@Test
	public void expandArrayVariablesShouldPrefixRoot() {
		Object[] uriVariables = new Object[0];
		URI expanded = this.handler.expand("/hello", uriVariables);
		verify(this.delegate).expand("http:		assertThat(expanded).isEqualTo(this.uri);
	}

	@Test
	public void expandArrayVariablesWhenPathDoesNotStartWithSlashShouldNotPrefixRoot() {
		Object[] uriVariables = new Object[0];
		URI expanded = this.handler.expand("http:		verify(this.delegate).expand("http:		assertThat(expanded).isEqualTo(this.uri);
	}

	@Test
	public void applyShouldWrapExistingTemplate() {
		RestTemplate restTemplate = new RestTemplate();
		restTemplate.setUriTemplateHandler(this.delegate);
		this.handler = RootUriTemplateHandler.addTo(restTemplate, "http:		Object[] uriVariables = new Object[0];
		URI expanded = this.handler.expand("/hello", uriVariables);
		verify(this.delegate).expand("http:		assertThat(expanded).isEqualTo(this.uri);
	}

}
