

package org.springframework.boot.cli.command.init;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;


class ProjectType {

	private final String id;

	private final String name;

	private final String action;

	private final boolean defaultType;

	private final Map<String, String> tags = new HashMap<>();

	ProjectType(String id, String name, String action, boolean defaultType,
			Map<String, String> tags) {
		this.id = id;
		this.name = name;
		this.action = action;
		this.defaultType = defaultType;
		if (tags != null) {
			this.tags.putAll(tags);
		}
	}

	public String getId() {
		return this.id;
	}

	public String getName() {
		return this.name;
	}

	public String getAction() {
		return this.action;
	}

	public boolean isDefaultType() {
		return this.defaultType;
	}

	public Map<String, String> getTags() {
		return Collections.unmodifiableMap(this.tags);
	}

}
