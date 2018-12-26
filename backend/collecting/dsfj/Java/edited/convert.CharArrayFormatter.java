

package org.springframework.boot.convert;

import java.text.ParseException;
import java.util.Locale;

import org.springframework.format.Formatter;


final class CharArrayFormatter implements Formatter<char[]> {

	@Override
	public String print(char[] object, Locale locale) {
		return new String(object);
	}

	@Override
	public char[] parse(String text, Locale locale) throws ParseException {
		return text.toCharArray();
	}

}
