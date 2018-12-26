

package org.springframework.boot.configurationprocessor.fieldvalues;

import java.util.Collections;
import java.util.Map;

import javax.lang.model.element.TypeElement;

import org.springframework.boot.configurationprocessor.fieldvalues.javac.JavaCompilerFieldValuesParser;


@FunctionalInterface
public interface FieldValuesParser {

	
	FieldValuesParser NONE = (element) -> Collections.emptyMap();

	
	Map<String, Object> getFieldValues(TypeElement element) throws Exception;

}
