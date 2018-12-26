

package com.badlogic.gdx.net;

import java.io.InputStream;
import java.util.Map;

import com.badlogic.gdx.Net.HttpRequest;
import com.badlogic.gdx.utils.Base64Coder;
import com.badlogic.gdx.utils.Json;
import com.badlogic.gdx.utils.Pools;


public class HttpRequestBuilder {

	
	public static String baseUrl = "";

	
	public static int defaultTimeout = 1000;

	
	public static Json json = new Json();

	private HttpRequest httpRequest;

	
	public HttpRequestBuilder newRequest () {
		if (httpRequest != null) {
			throw new IllegalStateException("A new request has already been started. Call HttpRequestBuilder.build() first.");
		}

		httpRequest = Pools.obtain(HttpRequest.class);
		httpRequest.setTimeOut(defaultTimeout);
		return this;
	}

	
	public HttpRequestBuilder method (String httpMethod) {
		validate();
		httpRequest.setMethod(httpMethod);
		return this;
	}

	
	public HttpRequestBuilder url (String url) {
		validate();
		httpRequest.setUrl(baseUrl + url);
		return this;
	}

	
	public HttpRequestBuilder timeout (int timeOut) {
		validate();
		httpRequest.setTimeOut(timeOut);
		return this;
	}

	
	public HttpRequestBuilder followRedirects (boolean followRedirects) {
		validate();
		httpRequest.setFollowRedirects(followRedirects);
		return this;
	}
	
	
	public HttpRequestBuilder includeCredentials (boolean includeCredentials) {
		validate();
		httpRequest.setIncludeCredentials(includeCredentials);
		return this;
	}

	
	public HttpRequestBuilder header (String name, String value) {
		validate();
		httpRequest.setHeader(name, value);
		return this;
	}

	
	public HttpRequestBuilder content (String content) {
		validate();
		httpRequest.setContent(content);
		return this;
	}

	
	public HttpRequestBuilder content (InputStream contentStream, long contentLength) {
		validate();
		httpRequest.setContent(contentStream, contentLength);
		return this;
	}

	
	public HttpRequestBuilder formEncodedContent (Map<String, String> content) {
		validate();
		httpRequest.setHeader(HttpRequestHeader.ContentType, "application/x-www-form-urlencoded");
		String formEncodedContent = HttpParametersUtils.convertHttpParameters(content);
		httpRequest.setContent(formEncodedContent);
		return this;
	}

	
	public HttpRequestBuilder jsonContent (Object content) {
		validate();
		httpRequest.setHeader(HttpRequestHeader.ContentType, "application/json");
		String jsonContent = json.toJson(content);
		httpRequest.setContent(jsonContent);
		return this;
	}

	
	public HttpRequestBuilder basicAuthentication (String username, String password) {
		validate();
		httpRequest.setHeader(HttpRequestHeader.Authorization, "Basic " + Base64Coder.encodeString(username + ":" + password));
		return this;
	}

	
	public HttpRequest build () {
		validate();
		HttpRequest request = httpRequest;
		httpRequest = null;
		return request;
	}

	private void validate () {
		if (httpRequest == null) {
			throw new IllegalStateException("A new request has not been started yet. Call HttpRequestBuilder.newRequest() first.");
		}
	}

}
