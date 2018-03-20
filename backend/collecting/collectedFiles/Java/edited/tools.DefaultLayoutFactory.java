

package org.springframework.boot.loader.tools;

import java.io.File;


public class DefaultLayoutFactory implements LayoutFactory {

	@Override
	public Layout getLayout(File source) {
		return Layouts.forFile(source);
	}

}
