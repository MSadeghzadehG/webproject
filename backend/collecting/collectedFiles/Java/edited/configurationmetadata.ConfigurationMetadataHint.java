

package org.springframework.boot.configurationmetadata;

import java.util.ArrayList;
import java.util.List;


class ConfigurationMetadataHint {

	private static final String KEY_SUFFIX = ".keys";

	private static final String VALUE_SUFFIX = ".values";

	private String id;

	private final List<ValueHint> valueHints = new ArrayList<>();

	private final List<ValueProvider> valueProviders = new ArrayList<>();

	public boolean isMapKeyHints() {
		return (this.id != null && this.id.endsWith(KEY_SUFFIX));
	}

	public boolean isMapValueHints() {
		return (this.id != null && this.id.endsWith(VALUE_SUFFIX));
	}

	public String resolveId() {
		if (isMapKeyHints()) {
			return this.id.substring(0, this.id.length() - KEY_SUFFIX.length());
		}
		if (isMapValueHints()) {
			return this.id.substring(0, this.id.length() - VALUE_SUFFIX.length());
		}
		return this.id;
	}

	public String getId() {
		return this.id;
	}

	public void setId(String id) {
		this.id = id;
	}

	public List<ValueHint> getValueHints() {
		return this.valueHints;
	}

	public List<ValueProvider> getValueProviders() {
		return this.valueProviders;
	}

}
