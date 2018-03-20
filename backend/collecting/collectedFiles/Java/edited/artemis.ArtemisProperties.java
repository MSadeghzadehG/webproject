

package org.springframework.boot.autoconfigure.jms.artemis;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicInteger;

import org.apache.activemq.artemis.core.remoting.impl.invm.TransportConstants;

import org.springframework.boot.context.properties.ConfigurationProperties;


@ConfigurationProperties(prefix = "spring.artemis")
public class ArtemisProperties {

	
	private ArtemisMode mode;

	
	private String host = "localhost";

	
	private int port = 61616;

	
	private String user;

	
	private String password;

	private final Embedded embedded = new Embedded();

	public ArtemisMode getMode() {
		return this.mode;
	}

	public void setMode(ArtemisMode mode) {
		this.mode = mode;
	}

	public String getHost() {
		return this.host;
	}

	public void setHost(String host) {
		this.host = host;
	}

	public int getPort() {
		return this.port;
	}

	public void setPort(int port) {
		this.port = port;
	}

	public String getUser() {
		return this.user;
	}

	public void setUser(String user) {
		this.user = user;
	}

	public String getPassword() {
		return this.password;
	}

	public void setPassword(String password) {
		this.password = password;
	}

	public Embedded getEmbedded() {
		return this.embedded;
	}

	
	public static class Embedded {

		private static final AtomicInteger serverIdCounter = new AtomicInteger();

		
		private int serverId = serverIdCounter.getAndIncrement();

		
		private boolean enabled = true;

		
		private boolean persistent;

		
		private String dataDirectory;

		
		private String[] queues = new String[0];

		
		private String[] topics = new String[0];

		
		private String clusterPassword = UUID.randomUUID().toString();

		private boolean defaultClusterPassword = true;

		public int getServerId() {
			return this.serverId;
		}

		public void setServerId(int serverId) {
			this.serverId = serverId;
		}

		public boolean isEnabled() {
			return this.enabled;
		}

		public void setEnabled(boolean enabled) {
			this.enabled = enabled;
		}

		public boolean isPersistent() {
			return this.persistent;
		}

		public void setPersistent(boolean persistent) {
			this.persistent = persistent;
		}

		public String getDataDirectory() {
			return this.dataDirectory;
		}

		public void setDataDirectory(String dataDirectory) {
			this.dataDirectory = dataDirectory;
		}

		public String[] getQueues() {
			return this.queues;
		}

		public void setQueues(String[] queues) {
			this.queues = queues;
		}

		public String[] getTopics() {
			return this.topics;
		}

		public void setTopics(String[] topics) {
			this.topics = topics;
		}

		public String getClusterPassword() {
			return this.clusterPassword;
		}

		public void setClusterPassword(String clusterPassword) {
			this.clusterPassword = clusterPassword;
			this.defaultClusterPassword = false;
		}

		public boolean isDefaultClusterPassword() {
			return this.defaultClusterPassword;
		}

		
		public Map<String, Object> generateTransportParameters() {
			Map<String, Object> parameters = new HashMap<>();
			parameters.put(TransportConstants.SERVER_ID_PROP_NAME, getServerId());
			return parameters;
		}

	}

}
