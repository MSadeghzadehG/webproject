

package org.springframework.boot.configurationmetadata;

import java.util.ArrayList;
import java.util.List;


public class Hints {

	private final List<ValueHint> keyHints = new ArrayList<>();

	private final List<ValueProvider> keyProviders = new ArrayList<>();

	private final List<ValueHint> valueHints = new ArrayList<>();

	private final List<ValueProvider> valueProviders = new ArrayList<>();

	
	public List<ValueHint> getKeyHints() {
		return this.keyHints;
	}

	
	public List<ValueProvider> getKeyProviders() {
		return this.keyProviders;
	}

	
	public List<ValueHint> getValueHints() {
		return this.valueHints;
	}

	
	public List<ValueProvider> getValueProviders() {
		return this.valueProviders;
	}

}
