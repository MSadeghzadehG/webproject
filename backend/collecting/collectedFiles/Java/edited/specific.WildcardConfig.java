

package org.springframework.boot.configurationsample.specific;

import java.util.List;
import java.util.Map;

import org.springframework.boot.configurationsample.ConfigurationProperties;


@ConfigurationProperties("wildcard")
public class WildcardConfig {

	private Map<String, ? extends Number> stringToNumber;

	private List<? super Integer> integers;

	public Map<String, ? extends Number> getStringToNumber() {
		return this.stringToNumber;
	}

	public void setStringToNumber(Map<String, ? extends Number> stringToNumber) {
		this.stringToNumber = stringToNumber;
	}

	public List<? super Integer> getIntegers() {
		return this.integers;
	}

	public void setIntegers(List<? super Integer> integers) {
		this.integers = integers;
	}

}
