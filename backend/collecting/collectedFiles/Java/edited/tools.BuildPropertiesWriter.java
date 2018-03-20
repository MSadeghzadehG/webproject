

package org.springframework.boot.loader.tools;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.time.Instant;
import java.time.format.DateTimeFormatter;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Properties;


public final class BuildPropertiesWriter {

	private final File outputFile;

	
	public BuildPropertiesWriter(File outputFile) {
		this.outputFile = outputFile;
	}

	public void writeBuildProperties(ProjectDetails projectDetails) throws IOException {
		Properties properties = createBuildInfo(projectDetails);
		createFileIfNecessary(this.outputFile);
		try (FileOutputStream outputStream = new FileOutputStream(this.outputFile)) {
			properties.store(outputStream, "Properties");
		}
	}

	private void createFileIfNecessary(File file) throws IOException {
		if (file.exists()) {
			return;
		}
		File parent = file.getParentFile();
		if (!parent.isDirectory() && !parent.mkdirs()) {
			throw new IllegalStateException("Cannot create parent directory for '"
					+ this.outputFile.getAbsolutePath() + "'");
		}
		if (!file.createNewFile()) {
			throw new IllegalStateException("Cannot create target file '"
					+ this.outputFile.getAbsolutePath() + "'");
		}
	}

	protected Properties createBuildInfo(ProjectDetails project) {
		Properties properties = new Properties();
		properties.put("build.group", project.getGroup());
		properties.put("build.artifact", project.getArtifact());
		properties.put("build.name", project.getName());
		properties.put("build.version", project.getVersion());
		if (project.getTime() != null) {
			properties.put("build.time",
					DateTimeFormatter.ISO_INSTANT.format(project.getTime()));
		}
		if (project.getAdditionalProperties() != null) {
			for (Map.Entry<String, String> entry : project.getAdditionalProperties()
					.entrySet()) {
				properties.put("build." + entry.getKey(), entry.getValue());
			}
		}
		return properties;
	}

	
	public static final class ProjectDetails {

		private final String group;

		private final String artifact;

		private final String name;

		private final String version;

		private final Instant time;

		private final Map<String, String> additionalProperties;

		public ProjectDetails(String group, String artifact, String version, String name,
				Instant time, Map<String, String> additionalProperties) {
			this.group = group;
			this.artifact = artifact;
			this.name = name;
			this.version = version;
			this.time = time;
			validateAdditionalProperties(additionalProperties);
			this.additionalProperties = additionalProperties;
		}

		private static void validateAdditionalProperties(
				Map<String, String> additionalProperties) {
			if (additionalProperties != null) {
				for (Entry<String, String> property : additionalProperties.entrySet()) {
					if (property.getValue() == null) {
						throw new NullAdditionalPropertyValueException(property.getKey());
					}
				}
			}
		}

		public String getGroup() {
			return this.group;
		}

		public String getArtifact() {
			return this.artifact;
		}

		public String getName() {
			return this.name;
		}

		public String getVersion() {
			return this.version;
		}

		public Instant getTime() {
			return this.time;
		}

		public Map<String, String> getAdditionalProperties() {
			return this.additionalProperties;
		}

	}

	
	public static class NullAdditionalPropertyValueException
			extends IllegalArgumentException {

		public NullAdditionalPropertyValueException(String name) {
			super("Additional property '" + name + "' is illegal as its value is null");
		}

	}

}
