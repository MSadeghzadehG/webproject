

package org.springframework.boot.actuate.endpoint.invoke.reflect;

import java.lang.reflect.Method;
import java.security.Principal;
import java.util.Set;
import java.util.stream.Collectors;

import org.springframework.boot.actuate.endpoint.InvocationContext;
import org.springframework.boot.actuate.endpoint.SecurityContext;
import org.springframework.boot.actuate.endpoint.invoke.MissingParametersException;
import org.springframework.boot.actuate.endpoint.invoke.OperationInvoker;
import org.springframework.boot.actuate.endpoint.invoke.OperationParameter;
import org.springframework.boot.actuate.endpoint.invoke.ParameterValueMapper;
import org.springframework.core.style.ToStringCreator;
import org.springframework.util.Assert;
import org.springframework.util.ReflectionUtils;


public class ReflectiveOperationInvoker implements OperationInvoker {

	private final Object target;

	private final OperationMethod operationMethod;

	private final ParameterValueMapper parameterValueMapper;

	
	public ReflectiveOperationInvoker(Object target, OperationMethod operationMethod,
			ParameterValueMapper parameterValueMapper) {
		Assert.notNull(target, "Target must not be null");
		Assert.notNull(operationMethod, "OperationMethod must not be null");
		Assert.notNull(parameterValueMapper, "ParameterValueMapper must not be null");
		ReflectionUtils.makeAccessible(operationMethod.getMethod());
		this.target = target;
		this.operationMethod = operationMethod;
		this.parameterValueMapper = parameterValueMapper;
	}

	@Override
	public Object invoke(InvocationContext context) {
		validateRequiredParameters(context);
		Method method = this.operationMethod.getMethod();
		Object[] resolvedArguments = resolveArguments(context);
		ReflectionUtils.makeAccessible(method);
		return ReflectionUtils.invokeMethod(method, this.target, resolvedArguments);
	}

	private void validateRequiredParameters(InvocationContext context) {
		Set<OperationParameter> missing = this.operationMethod.getParameters().stream()
				.filter((parameter) -> isMissing(context, parameter))
				.collect(Collectors.toSet());
		if (!missing.isEmpty()) {
			throw new MissingParametersException(missing);
		}
	}

	private boolean isMissing(InvocationContext context, OperationParameter parameter) {
		if (!parameter.isMandatory()) {
			return false;
		}
		if (Principal.class.equals(parameter.getType())) {
			return context.getSecurityContext().getPrincipal() == null;
		}
		if (SecurityContext.class.equals(parameter.getType())) {
			return false;
		}
		return context.getArguments().get(parameter.getName()) == null;
	}

	private Object[] resolveArguments(InvocationContext context) {
		return this.operationMethod.getParameters().stream()
				.map((parameter) -> resolveArgument(parameter, context)).toArray();
	}

	private Object resolveArgument(OperationParameter parameter,
			InvocationContext context) {
		if (Principal.class.equals(parameter.getType())) {
			return context.getSecurityContext().getPrincipal();
		}
		if (SecurityContext.class.equals(parameter.getType())) {
			return context.getSecurityContext();
		}
		Object value = context.getArguments().get(parameter.getName());
		return this.parameterValueMapper.mapParameterValue(parameter, value);
	}

	@Override
	public String toString() {
		return new ToStringCreator(this).append("target", this.target)
				.append("method", this.operationMethod).toString();
	}

}
