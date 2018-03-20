

package org.springframework.boot.actuate.health;

import java.util.Locale;
import java.util.function.Function;


public class HealthIndicatorNameFactory implements Function<String, String> {

	@Override
	public String apply(String name) {
		int index = name.toLowerCase(Locale.ENGLISH).indexOf("healthindicator");
		if (index > 0) {
			return name.substring(0, index);
		}
		return name;
	}

}
