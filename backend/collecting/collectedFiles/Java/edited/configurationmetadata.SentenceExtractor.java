

package org.springframework.boot.configurationmetadata;

import java.text.BreakIterator;
import java.util.Locale;


class SentenceExtractor {

	public String getFirstSentence(String text) {
		if (text == null) {
			return null;
		}
		int dot = text.indexOf('.');
		if (dot != -1) {
			BreakIterator breakIterator = BreakIterator.getSentenceInstance(Locale.US);
			breakIterator.setText(text);
			String sentence = text.substring(breakIterator.first(), breakIterator.next());
			return removeSpaceBetweenLine(sentence.trim());
		}
		else {
			String[] lines = text.split(System.lineSeparator());
			return lines[0].trim();
		}
	}

	private String removeSpaceBetweenLine(String text) {
		String[] lines = text.split(System.lineSeparator());
		StringBuilder sb = new StringBuilder();
		for (String line : lines) {
			sb.append(line.trim()).append(" ");
		}
		return sb.toString().trim();
	}

}
