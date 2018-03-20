

package org.springframework.boot.autoconfigure.couchbase;

import com.couchbase.client.java.Bucket;
import com.couchbase.client.java.Cluster;
import com.couchbase.client.java.CouchbaseBucket;
import com.couchbase.client.java.cluster.ClusterInfo;
import com.couchbase.client.java.env.CouchbaseEnvironment;

import org.springframework.data.couchbase.config.CouchbaseConfigurer;
import org.springframework.stereotype.Component;

import static org.mockito.Mockito.mock;


@Component
public class CouchbaseTestConfigurer implements CouchbaseConfigurer {

	@Override
	public CouchbaseEnvironment couchbaseEnvironment() throws Exception {
		return mock(CouchbaseEnvironment.class);
	}

	@Override
	public Cluster couchbaseCluster() throws Exception {
		return mock(Cluster.class);
	}

	@Override
	public ClusterInfo couchbaseClusterInfo() {
		return mock(ClusterInfo.class);
	}

	@Override
	public Bucket couchbaseClient() {
		return mock(CouchbaseBucket.class);
	}

}
