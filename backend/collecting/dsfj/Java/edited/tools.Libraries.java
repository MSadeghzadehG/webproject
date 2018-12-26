

package org.springframework.boot.loader.tools;

import java.io.IOException;


@FunctionalInterface
public interface Libraries {

	
	Libraries NONE = (callback) -> {
	};

	
	void doWithLibraries(LibraryCallback callback) throws IOException;

}
