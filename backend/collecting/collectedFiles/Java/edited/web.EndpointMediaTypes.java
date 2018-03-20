

package org.springframework.boot.actuate.endpoint.web;

import java.util.Collections;
import java.util.List;

import org.springframework.util.Assert;


public class EndpointMediaTypes {

	private final List<String> produced;

	private final List<String> consumed;

	
	public EndpointMediaTypes(List<String> produced, List<String> consumed) {
		Assert.notNull(produced, "Produced must not be null");
		Assert.notNull(consumed, "Consumed must not be null");
		this.produced = Collections.unmodifiableList(produced);
		this.consumed = Collections.unmodifiableList(consumed);
	}

	
	public List<String> getProduced() {
		return this.produced;
	}

	
	public List<String> getConsumed() {
		return this.consumed;
	}

}
