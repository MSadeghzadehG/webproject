

package org.springframework.boot.loader.tools;

import java.io.File;
import java.io.IOException;


@FunctionalInterface
public interface LibraryCallback {

	
	void library(Library library) throws IOException;

}
