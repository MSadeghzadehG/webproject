

package org.springframework.boot.loader.tools;

import java.io.IOException;
import java.io.InputStream;


public interface LoaderClassesWriter {

	
	void writeLoaderClasses() throws IOException;

	
	void writeLoaderClasses(String loaderJarResourceName) throws IOException;

	
	void writeEntry(String name, InputStream inputStream) throws IOException;

}
