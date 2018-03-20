

package org.springframework.boot.actuate.couchbase;

import com.couchbase.client.java.bucket.BucketInfo;
import com.couchbase.client.java.cluster.ClusterInfo;

import org.springframework.boot.actuate.health.AbstractHealthIndicator;
import org.springframework.boot.actuate.health.Health;
import org.springframework.boot.actuate.health.HealthIndicator;
import org.springframework.data.couchbase.core.CouchbaseOperations;
import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


public class CouchbaseHealthIndicator extends AbstractHealthIndicator {

	private CouchbaseOperations operations;

	public CouchbaseHealthIndicator() {
		super("Couchbase health check failed");
	}

	public CouchbaseHealthIndicator(CouchbaseOperations couchbaseOperations) {
		super("Couchbase health check failed");
		Assert.notNull(couchbaseOperations, "CouchbaseOperations must not be null");
		this.operations = couchbaseOperations;
	}

	@Override
	protected void doHealthCheck(Health.Builder builder) throws Exception {
		ClusterInfo cluster = this.operations.getCouchbaseClusterInfo();
		BucketInfo bucket = this.operations.getCouchbaseBucket().bucketManager().info();
		String versions = StringUtils
				.collectionToCommaDelimitedString(cluster.getAllVersions());
		String nodes = StringUtils.collectionToCommaDelimitedString(bucket.nodeList());
		builder.up().withDetail("versions", versions).withDetail("nodes", nodes);
	}

}
