
package org.elasticsearch.test.junit.annotations;

import com.carrotsearch.randomizedtesting.annotations.TestGroup;

import java.lang.annotation.Inherited;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;


@Retention(RetentionPolicy.RUNTIME)
@Inherited
@TestGroup(enabled = false, sysProperty = "tests.network")
public @interface Network {
}
