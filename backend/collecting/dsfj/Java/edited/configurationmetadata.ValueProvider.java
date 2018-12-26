

package org.springframework.boot.configurationmetadata;

import java.io.Serializable;
import java.util.LinkedHashMap;
import java.util.Map;


@SuppressWarnings("serial")
public class ValueProvider implements Serializable {

	private String name;

	private final Map<String, Object> parameters = new LinkedHashMap<>();

	
	public String getName() {
		return this.name;
	}

	public void setName(String name) {
		this.name = name;
	}

	
	public Map<String, Object> getParameters() {
		return this.parameters;
	}

	@Override
	public String toString() {
		return "ValueProvider{" + "name='" + this.name + ", parameters=" + this.parameters
				+ '}';
	}

}
