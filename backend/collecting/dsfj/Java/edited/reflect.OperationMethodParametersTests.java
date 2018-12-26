

package org.springframework.boot.actuate.endpoint.invoke.reflect;

import java.lang.reflect.Method;
import java.util.Iterator;
import java.util.List;
import java.util.Spliterator;
import java.util.Spliterators;
import java.util.stream.Collectors;
import java.util.stream.Stream;
import java.util.stream.StreamSupport;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import org.springframework.boot.actuate.endpoint.invoke.OperationParameter;
import org.springframework.core.DefaultParameterNameDiscoverer;
import org.springframework.core.ParameterNameDiscoverer;
import org.springframework.util.ReflectionUtils;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.Mockito.mock;


public class OperationMethodParametersTests {

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	private Method exampleMethod = ReflectionUtils.findMethod(getClass(), "example",
			String.class);

	private Method exampleNoParamsMethod = ReflectionUtils.findMethod(getClass(),
			"exampleNoParams");

	@Test
	public void createWhenMethodIsNullShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("Method must not be null");
		new OperationMethodParameters(null, mock(ParameterNameDiscoverer.class));
	}

	@Test
	public void createWhenParameterNameDiscovererIsNullShouldThrowException() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("ParameterNameDiscoverer must not be null");
		new OperationMethodParameters(this.exampleMethod, null);
	}

	@Test
	public void createWhenParameterNameDiscovererReturnsNullShouldThrowException() {
		this.thrown.expect(IllegalStateException.class);
		this.thrown.expectMessage("Failed to extract parameter names");
		new OperationMethodParameters(this.exampleMethod,
				mock(ParameterNameDiscoverer.class));
	}

	@Test
	public void hasParametersWhenHasParametersShouldReturnTrue() {
		OperationMethodParameters parameters = new OperationMethodParameters(
				this.exampleMethod, new DefaultParameterNameDiscoverer());
		assertThat(parameters.hasParameters()).isTrue();
	}

	@Test
	public void hasParametersWhenHasNoParametersShouldReturnFalse() {
		OperationMethodParameters parameters = new OperationMethodParameters(
				this.exampleNoParamsMethod, new DefaultParameterNameDiscoverer());
		assertThat(parameters.hasParameters()).isFalse();
	}

	@Test
	public void getParameterCountShouldReturnParameterCount() {
		OperationMethodParameters parameters = new OperationMethodParameters(
				this.exampleMethod, new DefaultParameterNameDiscoverer());
		assertThat(parameters.getParameterCount()).isEqualTo(1);
	}

	@Test
	public void iteratorShouldIterateOperationParameters() {
		OperationMethodParameters parameters = new OperationMethodParameters(
				this.exampleMethod, new DefaultParameterNameDiscoverer());
		Iterator<OperationParameter> iterator = parameters.iterator();
		assertParameters(StreamSupport.stream(
				Spliterators.spliteratorUnknownSize(iterator, Spliterator.ORDERED),
				false));
	}

	@Test
	public void streamShouldStreamOperationParameters() {
		OperationMethodParameters parameters = new OperationMethodParameters(
				this.exampleMethod, new DefaultParameterNameDiscoverer());
		assertParameters(parameters.stream());
	}

	private void assertParameters(Stream<OperationParameter> stream) {
		List<OperationParameter> parameters = stream.collect(Collectors.toList());
		assertThat(parameters).hasSize(1);
		OperationParameter parameter = parameters.get(0);
		assertThat(parameter.getName()).isEqualTo("name");
		assertThat(parameter.getType()).isEqualTo(String.class);
	}

	String example(String name) {
		return name;
	}

	String exampleNoParams() {
		return "example";
	}

}
