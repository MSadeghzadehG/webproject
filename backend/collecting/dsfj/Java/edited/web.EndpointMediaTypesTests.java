

package org.springframework.boot.actuate.endpoint.web;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import static org.assertj.core.api.Assertions.assertThat;


public class EndpointMediaTypesTests {

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	@Test
	public void createWhenProducedIsNullShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("Produced must not be null");
		new EndpointMediaTypes(null, Collections.emptyList());
	}

	@Test
	public void createWhenConsumedIsNullShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("Consumed must not be null");
		new EndpointMediaTypes(Collections.emptyList(), null);
	}

	@Test
	public void getProducedShouldReturnProduced() {
		List<String> produced = Arrays.asList("a", "b", "c");
		EndpointMediaTypes types = new EndpointMediaTypes(produced,
				Collections.emptyList());
		assertThat(types.getProduced()).isEqualTo(produced);
	}

	@Test
	public void getConsumedShouldReturnConsumed() {
		List<String> consumed = Arrays.asList("a", "b", "c");
		EndpointMediaTypes types = new EndpointMediaTypes(Collections.emptyList(),
				consumed);
		assertThat(types.getConsumed()).isEqualTo(consumed);
	}

}
