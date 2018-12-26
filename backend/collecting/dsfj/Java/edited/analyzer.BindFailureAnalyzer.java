

package org.springframework.boot.diagnostics.analyzer;

import java.util.Collection;
import java.util.Collections;
import java.util.TreeSet;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import org.springframework.boot.context.properties.bind.BindException;
import org.springframework.boot.context.properties.bind.UnboundConfigurationPropertiesException;
import org.springframework.boot.context.properties.bind.validation.BindValidationException;
import org.springframework.boot.context.properties.source.ConfigurationProperty;
import org.springframework.boot.diagnostics.AbstractFailureAnalyzer;
import org.springframework.boot.diagnostics.FailureAnalysis;
import org.springframework.core.convert.ConversionFailedException;
import org.springframework.util.StringUtils;


class BindFailureAnalyzer extends AbstractFailureAnalyzer<BindException> {

	@Override
	protected FailureAnalysis analyze(Throwable rootFailure, BindException cause) {
		Throwable rootCause = cause.getCause();
		if (rootCause instanceof BindValidationException
				|| rootCause instanceof UnboundConfigurationPropertiesException) {
			return null;
		}
		return analyzeGenericBindException(cause);
	}

	private FailureAnalysis analyzeGenericBindException(BindException cause) {
		StringBuilder description = new StringBuilder(
				String.format("%s:%n", cause.getMessage()));
		ConfigurationProperty property = cause.getProperty();
		buildDescription(description, property);
		description.append(String.format("%n    Reason: %s", getMessage(cause)));
		return getFailureAnalysis(description, cause);
	}

	private void buildDescription(StringBuilder description,
			ConfigurationProperty property) {
		if (property != null) {
			description.append(String.format("%n    Property: %s", property.getName()));
			description.append(String.format("%n    Value: %s", property.getValue()));
			description.append(String.format("%n    Origin: %s", property.getOrigin()));
		}
	}

	private String getMessage(BindException cause) {
		if (cause.getCause() != null
				&& StringUtils.hasText(cause.getCause().getMessage())) {
			return cause.getCause().getMessage();
		}
		return cause.getMessage();
	}

	private FailureAnalysis getFailureAnalysis(Object description, BindException cause) {
		StringBuilder message = new StringBuilder(
				"Update your application's configuration");
		Collection<String> validValues = findValidValues(cause);
		if (!validValues.isEmpty()) {
			message.append(String.format(". The following values are valid:%n"));
			validValues
					.forEach((value) -> message.append(String.format("%n    %s", value)));
		}
		return new FailureAnalysis(description.toString(), message.toString(), cause);
	}

	private Collection<String> findValidValues(BindException ex) {
		ConversionFailedException conversionFailure = findCause(ex,
				ConversionFailedException.class);
		if (conversionFailure != null) {
			Object[] enumConstants = conversionFailure.getTargetType().getType()
					.getEnumConstants();
			if (enumConstants != null) {
				return Stream.of(enumConstants).map(Object::toString)
						.collect(Collectors.toCollection(TreeSet::new));
			}
		}
		return Collections.emptySet();
	}

}
