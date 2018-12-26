

package org.springframework.boot.context.embedded;

import java.io.File;
import java.util.Arrays;
import java.util.List;


class PackagedApplicationLauncher extends AbstractApplicationLauncher {

	PackagedApplicationLauncher(ApplicationBuilder applicationBuilder) {
		super(applicationBuilder);
	}

	@Override
	protected File getWorkingDirectory() {
		return null;
	}

	@Override
	protected String getDescription(String packaging) {
		return "packaged " + packaging;
	}

	@Override
	protected List<String> getArguments(File archive) {
		return Arrays.asList("-jar", archive.getAbsolutePath());
	}

}
