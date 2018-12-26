

package org.springframework.boot.cli.command.options;

import java.util.Set;


public interface OptionHelp {

	
	Set<String> getOptions();

	
	String getUsageHelp();

}
