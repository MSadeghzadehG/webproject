

package org.springframework.boot.autoconfigureprocessor;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.util.Map;
import java.util.Properties;

import javax.annotation.processing.SupportedAnnotationTypes;


@SupportedAnnotationTypes({
		"org.springframework.boot.autoconfigureprocessor.TestConfiguration",
		"org.springframework.boot.autoconfigureprocessor.TestConditionalOnClass",
		"org.springframework.boot.autoconfigureprocessor.TestAutoConfigureBefore",
		"org.springframework.boot.autoconfigureprocessor.TestAutoConfigureAfter",
		"org.springframework.boot.autoconfigureprocessor.TestAutoConfigureOrder" })
public class TestConditionMetadataAnnotationProcessor
		extends AutoConfigureAnnotationProcessor {

	private final File outputLocation;

	public TestConditionMetadataAnnotationProcessor(File outputLocation) {
		this.outputLocation = outputLocation;
	}

	@Override
	protected void addAnnotations(Map<String, String> annotations) {
		put(annotations, "Configuration", TestConfiguration.class);
		put(annotations, "ConditionalOnClass", TestConditionalOnClass.class);
		put(annotations, "AutoConfigureBefore", TestAutoConfigureBefore.class);
		put(annotations, "AutoConfigureAfter", TestAutoConfigureAfter.class);
		put(annotations, "AutoConfigureOrder", TestAutoConfigureOrder.class);
	}

	private void put(Map<String, String> annotations, String key, Class<?> value) {
		annotations.put(key, value.getName());
	}

	public Properties getWrittenProperties() throws IOException {
		File file = new File(this.outputLocation, PROPERTIES_PATH);
		if (!file.exists()) {
			return null;
		}
		try (FileInputStream inputStream = new FileInputStream(file)) {
			Properties properties = new Properties();
			properties.load(inputStream);
			return properties;
		}
	}

}
