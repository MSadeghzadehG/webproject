

package org.springframework.boot.loader.tools;

import java.io.File;


@FunctionalInterface
public interface LayoutFactory {

	
	Layout getLayout(File source);

}
