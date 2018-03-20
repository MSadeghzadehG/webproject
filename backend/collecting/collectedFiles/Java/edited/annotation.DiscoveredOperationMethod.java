

package org.springframework.boot.actuate.endpoint.annotation;

import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.springframework.boot.actuate.endpoint.OperationType;
import org.springframework.boot.actuate.endpoint.invoke.reflect.OperationMethod;
import org.springframework.core.annotation.AnnotationAttributes;
import org.springframework.util.Assert;


public class DiscoveredOperationMethod extends OperationMethod {

	private final List<String> producesMediaTypes;

	public DiscoveredOperationMethod(Method method, OperationType operationType,
			AnnotationAttributes annotationAttributes) {
		super(method, operationType);
		Assert.notNull(annotationAttributes, "AnnotationAttributes must not be null");
		String[] produces = annotationAttributes.getStringArray("produces");
		this.producesMediaTypes = Collections.unmodifiableList(Arrays.asList(produces));
	}

	public List<String> getProducesMediaTypes() {
		return this.producesMediaTypes;
	}

}
