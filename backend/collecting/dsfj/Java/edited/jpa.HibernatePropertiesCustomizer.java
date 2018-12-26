

package org.springframework.boot.autoconfigure.orm.jpa;

import java.util.Map;


@FunctionalInterface
public interface HibernatePropertiesCustomizer {

	
	void customize(Map<String, Object> hibernateProperties);

}
