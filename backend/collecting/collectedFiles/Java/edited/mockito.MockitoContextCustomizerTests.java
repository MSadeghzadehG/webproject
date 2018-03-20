

package org.springframework.boot.test.mock.mockito;

import java.util.Arrays;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.Set;

import org.junit.Test;

import org.springframework.boot.test.mock.mockito.example.ExampleService;
import org.springframework.boot.test.mock.mockito.example.ExampleServiceCaller;
import org.springframework.core.ResolvableType;

import static org.assertj.core.api.Assertions.assertThat;


public class MockitoContextCustomizerTests {

	private static final Set<MockDefinition> NO_DEFINITIONS = Collections.emptySet();

	@Test
	public void hashCodeAndEquals() {
		MockDefinition d1 = createTestMockDefinition(ExampleService.class);
		MockDefinition d2 = createTestMockDefinition(ExampleServiceCaller.class);
		MockitoContextCustomizer c1 = new MockitoContextCustomizer(NO_DEFINITIONS);
		MockitoContextCustomizer c2 = new MockitoContextCustomizer(
				new LinkedHashSet<>(Arrays.asList(d1, d2)));
		MockitoContextCustomizer c3 = new MockitoContextCustomizer(
				new LinkedHashSet<>(Arrays.asList(d2, d1)));
		assertThat(c2.hashCode()).isEqualTo(c3.hashCode());
		assertThat(c1).isEqualTo(c1).isNotEqualTo(c2);
		assertThat(c2).isEqualTo(c2).isEqualTo(c3).isNotEqualTo(c1);
	}

	private MockDefinition createTestMockDefinition(Class<?> typeToMock) {
		return new MockDefinition(null, ResolvableType.forClass(typeToMock), null, null,
				false, null, null);
	}

}
