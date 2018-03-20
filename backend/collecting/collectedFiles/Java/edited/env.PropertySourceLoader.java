

package org.springframework.boot.env;

import java.io.IOException;
import java.util.List;

import org.springframework.core.env.PropertySource;
import org.springframework.core.io.Resource;
import org.springframework.core.io.support.SpringFactoriesLoader;


public interface PropertySourceLoader {

	
	String[] getFileExtensions();

	
	List<PropertySource<?>> load(String name, Resource resource) throws IOException;

}
