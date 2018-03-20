

package org.springframework.boot.configurationmetadata;

import java.io.Serializable;


@SuppressWarnings("serial")
public class ConfigurationMetadataProperty implements Serializable {

	private String id;

	private String name;

	private String type;

	private String description;

	private String shortDescription;

	private Object defaultValue;

	private final Hints hints = new Hints();

	private Deprecation deprecation;

	
	public String getId() {
		return this.id;
	}

	public void setId(String id) {
		this.id = id;
	}

	
	public String getName() {
		return this.name;
	}

	public void setName(String name) {
		this.name = name;
	}

	
	public String getType() {
		return this.type;
	}

	public void setType(String type) {
		this.type = type;
	}

	
	public String getDescription() {
		return this.description;
	}

	public void setDescription(String description) {
		this.description = description;
	}

	
	public String getShortDescription() {
		return this.shortDescription;
	}

	public void setShortDescription(String shortDescription) {
		this.shortDescription = shortDescription;
	}

	
	public Object getDefaultValue() {
		return this.defaultValue;
	}

	public void setDefaultValue(Object defaultValue) {
		this.defaultValue = defaultValue;
	}

	
	public Hints getHints() {
		return this.hints;
	}

	
	public Deprecation getDeprecation() {
		return this.deprecation;
	}

	public void setDeprecation(Deprecation deprecation) {
		this.deprecation = deprecation;
	}

	
	public boolean isDeprecated() {
		return this.deprecation != null;
	}

}
