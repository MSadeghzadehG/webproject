

package org.springframework.boot.gradle.tasks.bundling;

import java.io.File;
import java.io.IOException;
import java.io.Serializable;
import java.util.HashMap;
import java.util.Map;

import org.springframework.boot.loader.tools.FileUtils;


@SuppressWarnings("serial")
public class LaunchScriptConfiguration implements Serializable {

	private final Map<String, String> properties = new HashMap<>();

	private File script;

	
	public Map<String, String> getProperties() {
		return this.properties;
	}

	
	public void properties(Map<String, String> properties) {
		this.properties.putAll(properties);
	}

	
	public File getScript() {
		return this.script;
	}

	
	public void setScript(File script) {
		this.script = script;
	}

	@Override
	public int hashCode() {
		final int prime = 31;
		int result = 1;
		result = prime * result
				+ ((this.properties == null) ? 0 : this.properties.hashCode());
		result = prime * result + ((this.script == null) ? 0 : this.script.hashCode());
		return result;
	}

	@Override
	public boolean equals(Object obj) {
		if (this == obj) {
			return true;
		}
		if (obj == null) {
			return false;
		}
		if (getClass() != obj.getClass()) {
			return false;
		}
		LaunchScriptConfiguration other = (LaunchScriptConfiguration) obj;
		if (!this.properties.equals(other.properties)) {
			return false;
		}
		if (this.script == null) {
			if (other.script != null) {
				return false;
			}
		}
		else if (!this.script.equals(other.script)) {
			return false;
		}
		else if (!equalContents(this.script, other.script)) {
			return false;
		}
		return true;
	}

	private boolean equalContents(File one, File two) {
		try {
			return FileUtils.sha1Hash(one).equals(FileUtils.sha1Hash(two));
		}
		catch (IOException ex) {
			return false;
		}
	}

}
