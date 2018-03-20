

package org.springframework.boot.test.autoconfigure.properties;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;


@Retention(RetentionPolicy.RUNTIME)
@PropertyMapping
@interface ExampleMapping {

	String exampleProperty();

}
