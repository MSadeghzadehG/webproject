

package org.springframework.boot.configurationprocessor.fieldvalues.javac;

import javax.annotation.processing.ProcessingEnvironment;

import org.springframework.boot.configurationprocessor.fieldvalues.AbstractFieldValuesProcessorTests;
import org.springframework.boot.configurationprocessor.fieldvalues.FieldValuesParser;

import static org.junit.Assume.assumeNoException;


public class JavaCompilerFieldValuesProcessorTests
		extends AbstractFieldValuesProcessorTests {

	@Override
	protected FieldValuesParser createProcessor(ProcessingEnvironment env) {
		try {
			return new JavaCompilerFieldValuesParser(env);
		}
		catch (Throwable ex) {
			assumeNoException(ex);
			throw new IllegalStateException();
		}
	}

}
