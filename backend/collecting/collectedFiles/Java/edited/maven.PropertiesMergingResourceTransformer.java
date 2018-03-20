

package org.springframework.boot.maven;

import java.io.IOException;
import java.io.InputStream;
import java.util.List;
import java.util.Map.Entry;
import java.util.Properties;
import java.util.jar.JarEntry;
import java.util.jar.JarOutputStream;

import org.apache.maven.plugins.shade.relocation.Relocator;
import org.apache.maven.plugins.shade.resource.ResourceTransformer;


public class PropertiesMergingResourceTransformer implements ResourceTransformer {

		private String resource;

	private final Properties data = new Properties();

	
	public Properties getData() {
		return this.data;
	}

	@Override
	public boolean canTransformResource(String resource) {
		if (this.resource != null && this.resource.equalsIgnoreCase(resource)) {
			return true;
		}
		return false;
	}

	@Override
	public void processResource(String resource, InputStream is,
			List<Relocator> relocators) throws IOException {
		Properties properties = new Properties();
		properties.load(is);
		is.close();
		for (Entry<Object, Object> entry : properties.entrySet()) {
			String name = (String) entry.getKey();
			String value = (String) entry.getValue();
			String existing = this.data.getProperty(name);
			this.data.setProperty(name,
					existing == null ? value : existing + "," + value);
		}
	}

	@Override
	public boolean hasTransformedResource() {
		return !this.data.isEmpty();
	}

	@Override
	public void modifyOutputStream(JarOutputStream os) throws IOException {
		os.putNextEntry(new JarEntry(this.resource));
		this.data.store(os, "Merged by PropertiesMergingResourceTransformer");
		os.flush();
		this.data.clear();
	}

	public String getResource() {
		return this.resource;
	}

	public void setResource(String resource) {
		this.resource = resource;
	}

}
