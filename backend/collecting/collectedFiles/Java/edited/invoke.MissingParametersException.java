

package org.springframework.boot.actuate.endpoint.invoke;

import java.util.Set;
import java.util.stream.Collectors;

import org.springframework.boot.actuate.endpoint.InvalidEndpointRequestException;
import org.springframework.util.StringUtils;


public final class MissingParametersException extends InvalidEndpointRequestException {

	private final Set<OperationParameter> missingParameters;

	public MissingParametersException(Set<OperationParameter> missingParameters) {
		super("Failed to invoke operation because the following required "
				+ "parameters were missing: "
				+ StringUtils.collectionToCommaDelimitedString(missingParameters),
				"Missing parameters: "
						+ missingParameters.stream().map(OperationParameter::getName)
								.collect(Collectors.joining(",")));
		this.missingParameters = missingParameters;
	}

	
	public Set<OperationParameter> getMissingParameters() {
		return this.missingParameters;
	}
}
