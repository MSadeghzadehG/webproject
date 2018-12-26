

package org.springframework.boot.autoconfigure.logging;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.stream.Collectors;

import org.springframework.boot.autoconfigure.condition.ConditionEvaluationReport;
import org.springframework.boot.autoconfigure.condition.ConditionEvaluationReport.ConditionAndOutcome;
import org.springframework.boot.autoconfigure.condition.ConditionEvaluationReport.ConditionAndOutcomes;
import org.springframework.util.ClassUtils;
import org.springframework.util.StringUtils;


public class ConditionEvaluationReportMessage {

	private StringBuilder message;

	public ConditionEvaluationReportMessage(ConditionEvaluationReport report) {
		this(report, "CONDITIONS EVALUATION REPORT");
	}

	public ConditionEvaluationReportMessage(ConditionEvaluationReport report,
			String title) {
		this.message = getLogMessage(report, title);
	}

	private StringBuilder getLogMessage(ConditionEvaluationReport report, String title) {
		StringBuilder message = new StringBuilder();
		message.append(String.format("%n%n%n"));
		StringBuilder separator = new StringBuilder();
		for (int i = 0; i < title.length(); i++) {
			separator.append("=");
		}
		message.append(String.format("%s%n", separator));
		message.append(String.format("%s%n", title));
		message.append(String.format("%s%n%n%n", separator));
		Map<String, ConditionAndOutcomes> shortOutcomes = orderByName(
				report.getConditionAndOutcomesBySource());
		logPositiveMatches(message, shortOutcomes);
		logNegativeMatches(message, shortOutcomes);
		logExclusions(report, message);
		logUnconditionalClasses(report, message);
		message.append(String.format("%n%n"));
		return message;
	}

	private void logPositiveMatches(StringBuilder message,
			Map<String, ConditionAndOutcomes> shortOutcomes) {
		message.append(String.format("Positive matches:%n"));
		message.append(String.format("-----------------%n"));
		List<Entry<String, ConditionAndOutcomes>> matched = shortOutcomes.entrySet()
				.stream().filter((entry) -> entry.getValue().isFullMatch())
				.collect(Collectors.toList());
		if (matched.isEmpty()) {
			message.append(String.format("%n    None%n"));
		}
		else {
			matched.forEach((entry) -> addMatchLogMessage(message, entry.getKey(),
					entry.getValue()));
		}
		message.append(String.format("%n%n"));
	}

	private void logNegativeMatches(StringBuilder message,
			Map<String, ConditionAndOutcomes> shortOutcomes) {
		message.append(String.format("Negative matches:%n"));
		message.append(String.format("-----------------%n"));
		List<Entry<String, ConditionAndOutcomes>> nonMatched = shortOutcomes.entrySet()
				.stream().filter((entry) -> !entry.getValue().isFullMatch())
				.collect(Collectors.toList());
		if (nonMatched.isEmpty()) {
			message.append(String.format("%n    None%n"));
		}
		else {
			nonMatched.forEach((entry) -> addNonMatchLogMessage(message, entry.getKey(),
					entry.getValue()));
		}
		message.append(String.format("%n%n"));
	}

	private void logExclusions(ConditionEvaluationReport report, StringBuilder message) {
		message.append(String.format("Exclusions:%n"));
		message.append(String.format("-----------%n"));
		if (report.getExclusions().isEmpty()) {
			message.append(String.format("%n    None%n"));
		}
		else {
			for (String exclusion : report.getExclusions()) {
				message.append(String.format("%n    %s%n", exclusion));
			}
		}
		message.append(String.format("%n%n"));
	}

	private void logUnconditionalClasses(ConditionEvaluationReport report,
			StringBuilder message) {
		message.append(String.format("Unconditional classes:%n"));
		message.append(String.format("----------------------%n"));
		if (report.getUnconditionalClasses().isEmpty()) {
			message.append(String.format("%n    None%n"));
		}
		else {
			for (String unconditionalClass : report.getUnconditionalClasses()) {
				message.append(String.format("%n    %s%n", unconditionalClass));
			}
		}
	}

	private Map<String, ConditionAndOutcomes> orderByName(
			Map<String, ConditionAndOutcomes> outcomes) {
		List<String> names = new ArrayList<>();
		Map<String, String> classNames = new HashMap<>();
		for (String name : outcomes.keySet()) {
			String shortName = ClassUtils.getShortName(name);
			names.add(shortName);
			classNames.put(shortName, name);
		}
		Collections.sort(names);
		Map<String, ConditionAndOutcomes> result = new LinkedHashMap<>();
		for (String shortName : names) {
			result.put(shortName, outcomes.get(classNames.get(shortName)));
		}
		return result;
	}

	private void addMatchLogMessage(StringBuilder message, String source,
			ConditionAndOutcomes matches) {
		message.append(String.format("%n   %s matched:%n", source));
		for (ConditionAndOutcome match : matches) {
			logConditionAndOutcome(message, "      ", match);
		}
	}

	private void addNonMatchLogMessage(StringBuilder message, String source,
			ConditionAndOutcomes conditionAndOutcomes) {
		message.append(String.format("%n   %s:%n", source));
		List<ConditionAndOutcome> matches = new ArrayList<>();
		List<ConditionAndOutcome> nonMatches = new ArrayList<>();
		for (ConditionAndOutcome conditionAndOutcome : conditionAndOutcomes) {
			if (conditionAndOutcome.getOutcome().isMatch()) {
				matches.add(conditionAndOutcome);
			}
			else {
				nonMatches.add(conditionAndOutcome);
			}
		}
		message.append(String.format("      Did not match:%n"));
		for (ConditionAndOutcome nonMatch : nonMatches) {
			logConditionAndOutcome(message, "         ", nonMatch);
		}
		if (!matches.isEmpty()) {
			message.append(String.format("      Matched:%n"));
			for (ConditionAndOutcome match : matches) {
				logConditionAndOutcome(message, "         ", match);
			}
		}
	}

	private void logConditionAndOutcome(StringBuilder message, String indent,
			ConditionAndOutcome conditionAndOutcome) {
		message.append(String.format("%s- ", indent));
		String outcomeMessage = conditionAndOutcome.getOutcome().getMessage();
		if (StringUtils.hasLength(outcomeMessage)) {
			message.append(outcomeMessage);
		}
		else {
			message.append(conditionAndOutcome.getOutcome().isMatch() ? "matched"
					: "did not match");
		}
		message.append(" (");
		message.append(
				ClassUtils.getShortName(conditionAndOutcome.getCondition().getClass()));
		message.append(String.format(")%n"));
	}

	@Override
	public String toString() {
		return this.message.toString();
	}

}
