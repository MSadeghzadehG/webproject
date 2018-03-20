

package org.springframework.boot.testsupport.web.servlet;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.NoSuchElementException;

import javax.servlet.Filter;
import javax.servlet.FilterRegistration;
import javax.servlet.RequestDispatcher;
import javax.servlet.Servlet;
import javax.servlet.ServletContext;
import javax.servlet.ServletException;
import javax.servlet.ServletRegistration;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;


public abstract class MockServletWebServer {

	private ServletContext servletContext;

	private final Initializer[] initializers;

	private final List<RegisteredServlet> registeredServlets = new ArrayList<>();

	private final List<RegisteredFilter> registeredFilters = new ArrayList<>();

	private final int port;

	public MockServletWebServer(Initializer[] initializers, int port) {
		this.initializers = initializers;
		this.port = port;
		initialize();
	}

	private void initialize() {
		try {
			this.servletContext = mock(ServletContext.class);
			given(this.servletContext.addServlet(anyString(), any(Servlet.class)))
					.willAnswer((invocation) -> {
						RegisteredServlet registeredServlet = new RegisteredServlet(
								invocation.getArgument(1));
						MockServletWebServer.this.registeredServlets
								.add(registeredServlet);
						return registeredServlet.getRegistration();
					});
			given(this.servletContext.addFilter(anyString(), any(Filter.class)))
					.willAnswer((invocation) -> {
						RegisteredFilter registeredFilter = new RegisteredFilter(
								invocation.getArgument(1));
						MockServletWebServer.this.registeredFilters.add(registeredFilter);
						return registeredFilter.getRegistration();
					});
			final Map<String, String> initParameters = new HashMap<>();
			given(this.servletContext.setInitParameter(anyString(), anyString()))
					.will((invocation) -> {
						initParameters.put(invocation.getArgument(0),
								invocation.getArgument(1));
						return null;
					});
			given(this.servletContext.getInitParameterNames())
					.willReturn(Collections.enumeration(initParameters.keySet()));
			given(this.servletContext.getInitParameter(anyString())).willAnswer(
					(invocation) -> initParameters.get(invocation.getArgument(0)));
			given(this.servletContext.getAttributeNames())
					.willReturn(MockServletWebServer.emptyEnumeration());
			given(this.servletContext.getNamedDispatcher("default"))
					.willReturn(mock(RequestDispatcher.class));
			for (Initializer initializer : this.initializers) {
				initializer.onStartup(this.servletContext);
			}
		}
		catch (ServletException ex) {
			throw new RuntimeException(ex);
		}
	}

	public void stop() {
		this.servletContext = null;
		this.registeredServlets.clear();
	}

	public ServletContext getServletContext() {
		return this.servletContext;
	}

	public Servlet[] getServlets() {
		Servlet[] servlets = new Servlet[this.registeredServlets.size()];
		for (int i = 0; i < servlets.length; i++) {
			servlets[i] = this.registeredServlets.get(i).getServlet();
		}
		return servlets;
	}

	public RegisteredServlet getRegisteredServlet(int index) {
		return getRegisteredServlets().get(index);
	}

	public List<RegisteredServlet> getRegisteredServlets() {
		return this.registeredServlets;
	}

	public RegisteredFilter getRegisteredFilters(int index) {
		return getRegisteredFilters().get(index);
	}

	public List<RegisteredFilter> getRegisteredFilters() {
		return this.registeredFilters;
	}

	public int getPort() {
		return this.port;
	}

	@SuppressWarnings("unchecked")
	public static <T> Enumeration<T> emptyEnumeration() {
		return (Enumeration<T>) EmptyEnumeration.EMPTY_ENUMERATION;
	}

	private static class EmptyEnumeration<E> implements Enumeration<E> {

		static final MockServletWebServer.EmptyEnumeration<Object> EMPTY_ENUMERATION = new MockServletWebServer.EmptyEnumeration<>();

		@Override
		public boolean hasMoreElements() {
			return false;
		}

		@Override
		public E nextElement() {
			throw new NoSuchElementException();
		}

	}

	
	public static class RegisteredServlet {

		private final Servlet servlet;

		private final ServletRegistration.Dynamic registration;

		public RegisteredServlet(Servlet servlet) {
			this.servlet = servlet;
			this.registration = mock(ServletRegistration.Dynamic.class);
		}

		public ServletRegistration.Dynamic getRegistration() {
			return this.registration;
		}

		public Servlet getServlet() {
			return this.servlet;
		}

	}

	
	public static class RegisteredFilter {

		private final Filter filter;

		private final FilterRegistration.Dynamic registration;

		public RegisteredFilter(Filter filter) {
			this.filter = filter;
			this.registration = mock(FilterRegistration.Dynamic.class);
		}

		public FilterRegistration.Dynamic getRegistration() {
			return this.registration;
		}

		public Filter getFilter() {
			return this.filter;
		}

	}

	
	@FunctionalInterface
	protected interface Initializer {

		void onStartup(ServletContext context) throws ServletException;

	}

}
