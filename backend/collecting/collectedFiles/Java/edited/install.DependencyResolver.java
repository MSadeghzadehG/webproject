

package org.springframework.boot.cli.command.install;

import java.io.File;
import java.util.List;


@FunctionalInterface
interface DependencyResolver {

	
	List<File> resolve(List<String> artifactIdentifiers) throws Exception;

}
