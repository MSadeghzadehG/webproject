

package org.springframework.boot.autoconfigure.session;

import java.time.Duration;
import java.util.Arrays;
import java.util.HashSet;
import java.util.Set;

import org.springframework.beans.factory.ObjectProvider;
import org.springframework.boot.autoconfigure.web.ServerProperties;
import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.web.servlet.DispatcherType;
import org.springframework.boot.web.servlet.server.Session;
import org.springframework.session.web.http.SessionRepositoryFilter;


@ConfigurationProperties(prefix = "spring.session")
public class SessionProperties {

	
	private StoreType storeType;

	
	private final Duration timeout;

	private Servlet servlet = new Servlet();

	public SessionProperties(ObjectProvider<ServerProperties> serverProperties) {
		ServerProperties properties = serverProperties.getIfUnique();
		Session session = (properties == null ? null
				: properties.getServlet().getSession());
		this.timeout = (session == null ? null : session.getTimeout());
	}

	public StoreType getStoreType() {
		return this.storeType;
	}

	public void setStoreType(StoreType storeType) {
		this.storeType = storeType;
	}

	
	public Duration getTimeout() {
		return this.timeout;
	}

	public Servlet getServlet() {
		return this.servlet;
	}

	public void setServlet(Servlet servlet) {
		this.servlet = servlet;
	}

	
	public static class Servlet {

		
		private int filterOrder = SessionRepositoryFilter.DEFAULT_ORDER;

		
		private Set<DispatcherType> filterDispatcherTypes = new HashSet<>(Arrays.asList(
				DispatcherType.ASYNC, DispatcherType.ERROR, DispatcherType.REQUEST));

		public int getFilterOrder() {
			return this.filterOrder;
		}

		public void setFilterOrder(int filterOrder) {
			this.filterOrder = filterOrder;
		}

		public Set<DispatcherType> getFilterDispatcherTypes() {
			return this.filterDispatcherTypes;
		}

		public void setFilterDispatcherTypes(Set<DispatcherType> filterDispatcherTypes) {
			this.filterDispatcherTypes = filterDispatcherTypes;
		}

	}

}
