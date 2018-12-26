

package org.springframework.boot.test.context.filter;

import java.util.Arrays;
import java.util.Collection;

import org.junit.jupiter.api.DynamicNode;
import org.junit.jupiter.api.DynamicTest;
import org.junit.jupiter.api.TestFactory;

public class JupiterTestFactoryExample {

	@TestFactory
	public Collection<DynamicNode> testFactory() {
		return Arrays.asList(DynamicTest.dynamicTest("Some dynamic test", () -> {
					}));
	}

}
