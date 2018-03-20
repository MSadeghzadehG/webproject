

package org.springframework.boot.test.json;

import java.util.Collections;
import java.util.Map;

import org.assertj.core.api.AssertProvider;
import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;


public class ObjectContentAssertTests {

	private static final ExampleObject SOURCE = new ExampleObject();

	private static final ExampleObject DIFFERENT;

	static {
		DIFFERENT = new ExampleObject();
		DIFFERENT.setAge(123);
	}

	@Test
	public void isEqualToWhenObjectsAreEqualShouldPass() {
		assertThat(forObject(SOURCE)).isEqualTo(SOURCE);
	}

	@Test(expected = AssertionError.class)
	public void isEqualToWhenObjectsAreDifferentShouldFail() {
		assertThat(forObject(SOURCE)).isEqualTo(DIFFERENT);
	}

	@Test
	public void asArrayForArrayShouldReturnObjectArrayAssert() {
		ExampleObject[] source = new ExampleObject[] { SOURCE };
		assertThat(forObject(source)).asArray().containsExactly(SOURCE);
	}

	@Test(expected = AssertionError.class)
	public void asArrayForNonArrayShouldFail() {
		assertThat(forObject(SOURCE)).asArray();
	}

	@Test
	public void asMapForMapShouldReturnMapAssert() {
		Map<String, ExampleObject> source = Collections.singletonMap("a", SOURCE);
		assertThat(forObject(source)).asMap().containsEntry("a", SOURCE);
	}

	@Test(expected = AssertionError.class)
	public void asMapForNonMapShouldFail() {
		assertThat(forObject(SOURCE)).asMap();
	}

	private AssertProvider<ObjectContentAssert<Object>> forObject(Object source) {
		return () -> new ObjectContentAssert<>(source);
	}

}
