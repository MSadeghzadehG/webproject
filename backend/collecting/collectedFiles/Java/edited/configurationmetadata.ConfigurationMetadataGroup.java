

package org.springframework.boot.configurationmetadata;

import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;


@SuppressWarnings("serial")
public class ConfigurationMetadataGroup implements Serializable {

	private final String id;

	private final Map<String, ConfigurationMetadataSource> sources = new HashMap<>();

	private final Map<String, ConfigurationMetadataProperty> properties = new HashMap<>();

	public ConfigurationMetadataGroup(String id) {
		this.id = id;
	}

	
	public String getId() {
		return this.id;
	}

	
	public Map<String, ConfigurationMetadataSource> getSources() {
		return this.sources;
	}

	
	public Map<String, ConfigurationMetadataProperty> getProperties() {
		return this.properties;
	}

}
