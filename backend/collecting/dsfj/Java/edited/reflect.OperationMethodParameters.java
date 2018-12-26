

package org.springframework.boot.actuate.endpoint.invoke.reflect;

import java.lang.reflect.Method;
import java.lang.reflect.Parameter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;
import java.util.stream.Stream;

import org.springframework.boot.actuate.endpoint.invoke.OperationParameter;
import org.springframework.boot.actuate.endpoint.invoke.OperationParameters;
import org.springframework.core.ParameterNameDiscoverer;
import org.springframework.util.Assert;


class OperationMethodParameters implements OperationParameters {

	private final List<OperationParameter> operationParameters;

	
	OperationMethodParameters(Method method,
			ParameterNameDiscoverer parameterNameDiscoverer) {
		Assert.notNull(method, "Method must not be null");
		Assert.notNull(parameterNameDiscoverer,
				"ParameterNameDiscoverer must not be null");
		String[] parameterNames = parameterNameDiscoverer.getParameterNames(method);
		Parameter[] parameters = method.getParameters();
		Assert.state(parameterNames != null,
				"Failed to extract parameter names for " + method);
		this.operationParameters = getOperationParameters(parameters, parameterNames);
	}

	private List<OperationParameter> getOperationParameters(Parameter[] parameters,
			String[] names) {
		List<OperationParameter> operationParameters = new ArrayList<>(parameters.length);
		for (int i = 0; i < names.length; i++) {
			operationParameters
					.add(new OperationMethodParameter(names[i], parameters[i]));
		}
		return Collections.unmodifiableList(operationParameters);
	}

	@Override
	public int getParameterCount() {
		return this.operationParameters.size();
	}

	@Override
	public OperationParameter get(int index) {
		return this.operationParameters.get(index);
	}

	@Override
	public Iterator<OperationParameter> iterator() {
		return this.operationParameters.iterator();
	}

	@Override
	public Stream<OperationParameter> stream() {
		return this.operationParameters.stream();
	}

}
