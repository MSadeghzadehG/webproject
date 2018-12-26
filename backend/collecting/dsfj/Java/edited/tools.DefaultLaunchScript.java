

package org.springframework.boot.loader.tools;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class DefaultLaunchScript implements LaunchScript {

	private static final int BUFFER_SIZE = 4096;

	private static final Pattern PLACEHOLDER_PATTERN = Pattern
			.compile("\\{\\{(\\w+)(:.*?)?\\}\\}(?!\\})");

	private static final Set<String> FILE_PATH_KEYS = Collections
			.unmodifiableSet(new HashSet<>(Arrays.asList("inlinedConfScript")));

	private final String content;

	
	public DefaultLaunchScript(File file, Map<?, ?> properties) throws IOException {
		String content = loadContent(file);
		this.content = expandPlaceholders(content, properties);
	}

	private String loadContent(File file) throws IOException {
		if (file == null) {
			return loadContent(getClass().getResourceAsStream("launch.script"));
		}
		return loadContent(new FileInputStream(file));
	}

	private String loadContent(InputStream inputStream) throws IOException {
		try {
			ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
			copy(inputStream, outputStream);
			return new String(outputStream.toByteArray(), StandardCharsets.UTF_8);
		}
		finally {
			inputStream.close();
		}
	}

	private void copy(InputStream inputStream, OutputStream outputStream)
			throws IOException {
		byte[] buffer = new byte[BUFFER_SIZE];
		int bytesRead;
		while ((bytesRead = inputStream.read(buffer)) != -1) {
			outputStream.write(buffer, 0, bytesRead);
		}
		outputStream.flush();
	}

	private String expandPlaceholders(String content, Map<?, ?> properties)
			throws IOException {
		StringBuffer expanded = new StringBuffer();
		Matcher matcher = PLACEHOLDER_PATTERN.matcher(content);
		while (matcher.find()) {
			String name = matcher.group(1);
			final String value;
			String defaultValue = matcher.group(2);
			if (properties != null && properties.containsKey(name)) {
				Object propertyValue = properties.get(name);
				if (FILE_PATH_KEYS.contains(name)) {
					value = parseFilePropertyValue(propertyValue);
				}
				else {
					value = propertyValue.toString();
				}
			}
			else {
				value = (defaultValue == null ? matcher.group(0)
						: defaultValue.substring(1));
			}
			matcher.appendReplacement(expanded, value.replace("$", "\\$"));
		}
		matcher.appendTail(expanded);
		return expanded.toString();
	}

	private String parseFilePropertyValue(Object propertyValue) throws IOException {
		if (propertyValue instanceof File) {
			return loadContent((File) propertyValue);
		}
		return loadContent(new File(propertyValue.toString()));
	}

	@Override
	public byte[] toByteArray() {
		return this.content.getBytes(StandardCharsets.UTF_8);
	}

}
