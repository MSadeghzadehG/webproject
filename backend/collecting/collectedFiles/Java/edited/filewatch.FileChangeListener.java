

package org.springframework.boot.devtools.filewatch;

import java.util.Set;


@FunctionalInterface
public interface FileChangeListener {

	
	void onChange(Set<ChangedFiles> changeSet);

}
