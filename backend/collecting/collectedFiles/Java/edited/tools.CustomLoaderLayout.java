

package org.springframework.boot.loader.tools;

import java.io.IOException;


@FunctionalInterface
public interface CustomLoaderLayout {

	
	void writeLoadedClasses(LoaderClassesWriter writer) throws IOException;

}
