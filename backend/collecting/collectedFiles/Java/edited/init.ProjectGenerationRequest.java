

package org.springframework.boot.cli.command.init;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import org.apache.http.client.utils.URIBuilder;

import org.springframework.util.StringUtils;


class ProjectGenerationRequest {

	public static final String DEFAULT_SERVICE_URL = "https:
	private String serviceUrl = DEFAULT_SERVICE_URL;

	private String output;

	private boolean extract;

	private String groupId;

	private String artifactId;

	private String version;

	private String name;

	private String description;

	private String packageName;

	private String type;

	private String packaging;

	private String build;

	private String format;

	private boolean detectType;

	private String javaVersion;

	private String language;

	private String bootVersion;

	private List<String> dependencies = new ArrayList<>();

	
	public String getServiceUrl() {
		return this.serviceUrl;
	}

	public void setServiceUrl(String serviceUrl) {
		this.serviceUrl = serviceUrl;
	}

	
	public String getOutput() {
		return this.output;
	}

	public void setOutput(String output) {
		if (output != null && output.endsWith("/")) {
			this.output = output.substring(0, output.length() - 1);
			this.extract = true;
		}
		else {
			this.output = output;
		}
	}

	
	public boolean isExtract() {
		return this.extract;
	}

	public void setExtract(boolean extract) {
		this.extract = extract;
	}

	
	public String getGroupId() {
		return this.groupId;
	}

	public void setGroupId(String groupId) {
		this.groupId = groupId;
	}

	
	public String getArtifactId() {
		return this.artifactId;
	}

	public void setArtifactId(String artifactId) {
		this.artifactId = artifactId;
	}

	
	public String getVersion() {
		return this.version;
	}

	public void setVersion(String version) {
		this.version = version;
	}

	
	public String getName() {
		return this.name;
	}

	public void setName(String name) {
		this.name = name;
	}

	
	public String getDescription() {
		return this.description;
	}

	public void setDescription(String description) {
		this.description = description;
	}

	
	public String getPackageName() {
		return this.packageName;
	}

	public void setPackageName(String packageName) {
		this.packageName = packageName;
	}

	
	public String getType() {
		return this.type;
	}

	public void setType(String type) {
		this.type = type;
	}

	
	public String getPackaging() {
		return this.packaging;
	}

	public void setPackaging(String packaging) {
		this.packaging = packaging;
	}

	
	public String getBuild() {
		return this.build;
	}

	public void setBuild(String build) {
		this.build = build;
	}

	
	public String getFormat() {
		return this.format;
	}

	public void setFormat(String format) {
		this.format = format;
	}

	
	public boolean isDetectType() {
		return this.detectType;
	}

	public void setDetectType(boolean detectType) {
		this.detectType = detectType;
	}

	
	public String getJavaVersion() {
		return this.javaVersion;
	}

	public void setJavaVersion(String javaVersion) {
		this.javaVersion = javaVersion;
	}

	
	public String getLanguage() {
		return this.language;
	}

	public void setLanguage(String language) {
		this.language = language;
	}

	
	public String getBootVersion() {
		return this.bootVersion;
	}

	public void setBootVersion(String bootVersion) {
		this.bootVersion = bootVersion;
	}

	
	public List<String> getDependencies() {
		return this.dependencies;
	}

	
	URI generateUrl(InitializrServiceMetadata metadata) {
		try {
			URIBuilder builder = new URIBuilder(this.serviceUrl);
			StringBuilder sb = new StringBuilder();
			if (builder.getPath() != null) {
				sb.append(builder.getPath());
			}

			ProjectType projectType = determineProjectType(metadata);
			this.type = projectType.getId();
			sb.append(projectType.getAction());
			builder.setPath(sb.toString());

			if (!this.dependencies.isEmpty()) {
				builder.setParameter("dependencies",
						StringUtils.collectionToCommaDelimitedString(this.dependencies));
			}

			if (this.groupId != null) {
				builder.setParameter("groupId", this.groupId);
			}
			String resolvedArtifactId = resolveArtifactId();
			if (resolvedArtifactId != null) {
				builder.setParameter("artifactId", resolvedArtifactId);
			}
			if (this.version != null) {
				builder.setParameter("version", this.version);
			}
			if (this.name != null) {
				builder.setParameter("name", this.name);
			}
			if (this.description != null) {
				builder.setParameter("description", this.description);
			}
			if (this.packageName != null) {
				builder.setParameter("packageName", this.packageName);
			}
			if (this.type != null) {
				builder.setParameter("type", projectType.getId());
			}
			if (this.packaging != null) {
				builder.setParameter("packaging", this.packaging);
			}
			if (this.javaVersion != null) {
				builder.setParameter("javaVersion", this.javaVersion);
			}
			if (this.language != null) {
				builder.setParameter("language", this.language);
			}
			if (this.bootVersion != null) {
				builder.setParameter("bootVersion", this.bootVersion);
			}

			return builder.build();
		}
		catch (URISyntaxException e) {
			throw new ReportableException("Invalid service URL (" + e.getMessage() + ")");
		}
	}

	protected ProjectType determineProjectType(InitializrServiceMetadata metadata) {
		if (this.type != null) {
			ProjectType result = metadata.getProjectTypes().get(this.type);
			if (result == null) {
				throw new ReportableException(("No project type with id '" + this.type
						+ "' - check the service capabilities (--list)"));
			}
			return result;
		}
		else if (isDetectType()) {
			Map<String, ProjectType> types = new HashMap<>(metadata.getProjectTypes());
			if (this.build != null) {
				filter(types, "build", this.build);
			}
			if (this.format != null) {
				filter(types, "format", this.format);
			}
			if (types.size() == 1) {
				return types.values().iterator().next();
			}
			else if (types.isEmpty()) {
				throw new ReportableException("No type found with build '" + this.build
						+ "' and format '" + this.format
						+ "' check the service capabilities (--list)");
			}
			else {
				throw new ReportableException("Multiple types found with build '"
						+ this.build + "' and format '" + this.format
						+ "' use --type with a more specific value " + types.keySet());
			}
		}
		else {
			ProjectType defaultType = metadata.getDefaultType();
			if (defaultType == null) {
				throw new ReportableException(
						("No project type is set and no default is defined. "
								+ "Check the service capabilities (--list)"));
			}
			return defaultType;
		}
	}

	
	protected String resolveArtifactId() {
		if (this.artifactId != null) {
			return this.artifactId;
		}
		if (this.output != null) {
			int i = this.output.lastIndexOf('.');
			return (i == -1 ? this.output : this.output.substring(0, i));
		}
		return null;
	}

	private static void filter(Map<String, ProjectType> projects, String tag,
			String tagValue) {
		for (Iterator<Map.Entry<String, ProjectType>> it = projects.entrySet()
				.iterator(); it.hasNext();) {
			Map.Entry<String, ProjectType> entry = it.next();
			String value = entry.getValue().getTags().get(tag);
			if (!tagValue.equals(value)) {
				it.remove();
			}
		}
	}

}
