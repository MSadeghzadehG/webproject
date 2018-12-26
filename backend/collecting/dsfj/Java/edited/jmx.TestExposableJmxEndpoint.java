

package org.springframework.boot.actuate.endpoint.jmx;

import java.util.Arrays;
import java.util.Collection;


public class TestExposableJmxEndpoint implements ExposableJmxEndpoint {

	private final Collection<JmxOperation> operations;

	public TestExposableJmxEndpoint(JmxOperation... operations) {
		this(Arrays.asList(operations));
	}

	public TestExposableJmxEndpoint(Collection<JmxOperation> operations) {
		this.operations = operations;
	}

	@Override
	public String getId() {
		return "test";
	}

	@Override
	public boolean isEnableByDefault() {
		return true;
	}

	@Override
	public Collection<JmxOperation> getOperations() {
		return this.operations;
	}

}
