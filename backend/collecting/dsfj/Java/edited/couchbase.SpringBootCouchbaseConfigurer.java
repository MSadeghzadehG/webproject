

package org.springframework.boot.autoconfigure.data.couchbase;

import com.couchbase.client.java.Bucket;
import com.couchbase.client.java.Cluster;
import com.couchbase.client.java.cluster.ClusterInfo;
import com.couchbase.client.java.env.CouchbaseEnvironment;

import org.springframework.data.couchbase.config.CouchbaseConfigurer;


public class SpringBootCouchbaseConfigurer implements CouchbaseConfigurer {

	private final CouchbaseEnvironment env;

	private final Cluster cluster;

	private final ClusterInfo clusterInfo;

	private final Bucket bucket;

	public SpringBootCouchbaseConfigurer(CouchbaseEnvironment env, Cluster cluster,
			ClusterInfo clusterInfo, Bucket bucket) {
		this.env = env;
		this.cluster = cluster;
		this.clusterInfo = clusterInfo;
		this.bucket = bucket;
	}

	@Override
	public CouchbaseEnvironment couchbaseEnvironment() throws Exception {
		return this.env;
	}

	@Override
	public Cluster couchbaseCluster() throws Exception {
		return this.cluster;
	}

	@Override
	public ClusterInfo couchbaseClusterInfo() throws Exception {
		return this.clusterInfo;
	}

	@Override
	public Bucket couchbaseClient() throws Exception {
		return this.bucket;
	}

}
