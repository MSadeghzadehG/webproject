

package com.badlogic.gdx.utils;

import java.util.Locale;


public class TextFormatter {

	private StringBuilder buffer = new StringBuilder();

	public TextFormatter (Locale locale, boolean useMessageFormat) {
			}

	
	public String format (String pattern, Object... args) {
		buffer.setLength(0);
		boolean changed = false;
		int placeholder = -1;
		int patternLength = pattern.length();
		for (int i = 0; i < patternLength; ++i) {
			char ch = pattern.charAt(i);
			if (placeholder < 0) { 				if (ch == '{') {
					changed = true;
					if (i + 1 < patternLength && pattern.charAt(i + 1) == '{') {
						buffer.append(ch); 						++i;
					} else {
						placeholder = 0; 					}
				} else {
					buffer.append(ch);
				}
			} else { 				if (ch == '}') {
					if (placeholder >= args.length)
						throw new IllegalArgumentException("Argument index out of bounds: " + placeholder);
					if (pattern.charAt(i - 1) == '{')
						throw new IllegalArgumentException("Missing argument index after a left curly brace");
					if (args[placeholder] == null)
						buffer.append("null"); 					else
						buffer.append(args[placeholder].toString()); 					placeholder = -1; 				} else {
					if (ch < '0' || ch > '9')
						throw new IllegalArgumentException("Unexpected '" + ch + "' while parsing argument index");
					placeholder = placeholder * 10 + (ch - '0');
				}
			}
		}
		if (placeholder >= 0) throw new IllegalArgumentException("Unmatched braces in the pattern.");

		return changed ? buffer.toString() : pattern;
	}
}
