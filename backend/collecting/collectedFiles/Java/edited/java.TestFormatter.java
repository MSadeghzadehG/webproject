

package org.springframework.boot.logging.java;

import java.util.logging.Formatter;
import java.util.logging.LogRecord;


public class TestFormatter extends Formatter {

	@Override
	public String format(LogRecord record) {
		return String.format("foo: %s -- %s\n", record.getLoggerName(),
				record.getMessage());
	}

}
