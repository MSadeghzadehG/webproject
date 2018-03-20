

package org.springframework.boot.devtools.tests;


public interface ApplicationLauncher {

	LaunchedApplication launchApplication(JvmLauncher javaLauncher) throws Exception;

}
