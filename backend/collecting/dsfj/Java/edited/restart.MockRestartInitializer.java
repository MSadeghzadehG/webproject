

package org.springframework.boot.devtools.restart;

import java.net.URL;


public class MockRestartInitializer implements RestartInitializer {

	@Override
	public URL[] getInitialUrls(Thread thread) {
		return new URL[] {};
	}

}
