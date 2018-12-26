

package org.springframework.boot.autoconfigure.cassandra;

import java.time.Duration;
import java.time.temporal.ChronoUnit;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import com.datastax.driver.core.ConsistencyLevel;
import com.datastax.driver.core.ProtocolOptions;
import com.datastax.driver.core.ProtocolOptions.Compression;
import com.datastax.driver.core.QueryOptions;
import com.datastax.driver.core.policies.LoadBalancingPolicy;
import com.datastax.driver.core.policies.ReconnectionPolicy;
import com.datastax.driver.core.policies.RetryPolicy;

import org.springframework.boot.context.properties.ConfigurationProperties;
import org.springframework.boot.convert.DurationUnit;


@ConfigurationProperties(prefix = "spring.data.cassandra")
public class CassandraProperties {

	
	private String keyspaceName;

	
	private String clusterName;

	
	private final List<String> contactPoints = new ArrayList<>(
			Collections.singleton("localhost"));

	
	private int port = ProtocolOptions.DEFAULT_PORT;

	
	private String username;

	
	private String password;

	
	private Compression compression = Compression.NONE;

	
	private Class<? extends LoadBalancingPolicy> loadBalancingPolicy;

	
	private ConsistencyLevel consistencyLevel;

	
	private ConsistencyLevel serialConsistencyLevel;

	
	private int fetchSize = QueryOptions.DEFAULT_FETCH_SIZE;

	
	private Class<? extends ReconnectionPolicy> reconnectionPolicy;

	
	private Class<? extends RetryPolicy> retryPolicy;

	
	private Duration connectTimeout;

	
	private Duration readTimeout;

	
	private String schemaAction = "none";

	
	private boolean ssl = false;

	
	private final Pool pool = new Pool();

	public String getKeyspaceName() {
		return this.keyspaceName;
	}

	public void setKeyspaceName(String keyspaceName) {
		this.keyspaceName = keyspaceName;
	}

	public String getClusterName() {
		return this.clusterName;
	}

	public void setClusterName(String clusterName) {
		this.clusterName = clusterName;
	}

	public List<String> getContactPoints() {
		return this.contactPoints;
	}

	public int getPort() {
		return this.port;
	}

	public void setPort(int port) {
		this.port = port;
	}

	public String getUsername() {
		return this.username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	public String getPassword() {
		return this.password;
	}

	public void setPassword(String password) {
		this.password = password;
	}

	public Compression getCompression() {
		return this.compression;
	}

	public void setCompression(Compression compression) {
		this.compression = compression;
	}

	public Class<? extends LoadBalancingPolicy> getLoadBalancingPolicy() {
		return this.loadBalancingPolicy;
	}

	public void setLoadBalancingPolicy(
			Class<? extends LoadBalancingPolicy> loadBalancingPolicy) {
		this.loadBalancingPolicy = loadBalancingPolicy;
	}

	public ConsistencyLevel getConsistencyLevel() {
		return this.consistencyLevel;
	}

	public void setConsistencyLevel(ConsistencyLevel consistency) {
		this.consistencyLevel = consistency;
	}

	public ConsistencyLevel getSerialConsistencyLevel() {
		return this.serialConsistencyLevel;
	}

	public void setSerialConsistencyLevel(ConsistencyLevel serialConsistency) {
		this.serialConsistencyLevel = serialConsistency;
	}

	public int getFetchSize() {
		return this.fetchSize;
	}

	public void setFetchSize(int fetchSize) {
		this.fetchSize = fetchSize;
	}

	public Class<? extends ReconnectionPolicy> getReconnectionPolicy() {
		return this.reconnectionPolicy;
	}

	public void setReconnectionPolicy(
			Class<? extends ReconnectionPolicy> reconnectionPolicy) {
		this.reconnectionPolicy = reconnectionPolicy;
	}

	public Class<? extends RetryPolicy> getRetryPolicy() {
		return this.retryPolicy;
	}

	public void setRetryPolicy(Class<? extends RetryPolicy> retryPolicy) {
		this.retryPolicy = retryPolicy;
	}

	public Duration getConnectTimeout() {
		return this.connectTimeout;
	}

	public void setConnectTimeout(Duration connectTimeout) {
		this.connectTimeout = connectTimeout;
	}

	public Duration getReadTimeout() {
		return this.readTimeout;
	}

	public void setReadTimeout(Duration readTimeout) {
		this.readTimeout = readTimeout;
	}

	public boolean isSsl() {
		return this.ssl;
	}

	public void setSsl(boolean ssl) {
		this.ssl = ssl;
	}

	public String getSchemaAction() {
		return this.schemaAction;
	}

	public void setSchemaAction(String schemaAction) {
		this.schemaAction = schemaAction;
	}

	public Pool getPool() {
		return this.pool;
	}

	
	public static class Pool {

		
		@DurationUnit(ChronoUnit.SECONDS)
		private Duration idleTimeout = Duration.ofSeconds(120);

		
		private Duration poolTimeout = Duration.ofMillis(5000);

		
		@DurationUnit(ChronoUnit.SECONDS)
		private Duration heartbeatInterval = Duration.ofSeconds(30);

		
		private int maxQueueSize = 256;

		public Duration getIdleTimeout() {
			return this.idleTimeout;
		}

		public void setIdleTimeout(Duration idleTimeout) {
			this.idleTimeout = idleTimeout;
		}

		public Duration getPoolTimeout() {
			return this.poolTimeout;
		}

		public void setPoolTimeout(Duration poolTimeout) {
			this.poolTimeout = poolTimeout;
		}

		public Duration getHeartbeatInterval() {
			return this.heartbeatInterval;
		}

		public void setHeartbeatInterval(Duration heartbeatInterval) {
			this.heartbeatInterval = heartbeatInterval;
		}

		public int getMaxQueueSize() {
			return this.maxQueueSize;
		}

		public void setMaxQueueSize(int maxQueueSize) {
			this.maxQueueSize = maxQueueSize;
		}

	}

}
