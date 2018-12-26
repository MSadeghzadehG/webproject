

package com.badlogic.gdx.tests;

import com.badlogic.gdx.tests.utils.GdxTest;
import com.badlogic.gdx.utils.GdxRuntimeException;
import com.badlogic.gdx.utils.TimeUtils;


public class TimeUtilsTest extends GdxTest {
	final long oneMilliInNanos = 1000000;

	@Override
	public void create () {
				long now = TimeUtils.nanoTime();
		long nowConvertToMillis = TimeUtils.nanosToMillis(now);
		long nowConvertBackToNanos = TimeUtils.millisToNanos(nowConvertToMillis);

		assertEpsilonEqual(now, nowConvertBackToNanos, "Nano -> Millis conversion");

				long millis = TimeUtils.millis();
		long millisToNanos = TimeUtils.millisToNanos(millis);
		long nanosToMillis = TimeUtils.nanosToMillis(millisToNanos);

		assertAbsoluteEqual(millis, nanosToMillis, "Millis -> Nanos conversion");

				long oneSecondMillis = 1000;
		long oneSecondNanos = 1000000000;

		assertAbsoluteEqual(oneSecondMillis, TimeUtils.nanosToMillis(oneSecondNanos), "One Second Comparison, Nano -> Millis");
		assertAbsoluteEqual(TimeUtils.millisToNanos(oneSecondMillis), oneSecondNanos, "One Second Comparison, Millis -> Nanos");
	}

	@Override
	public void render () {

	}

	private void failTest (String testName) {
		throw new GdxRuntimeException("FAILED TEST: [" + testName + "]");
	}

	private void assertAbsoluteEqual (long a, long b, String testName) {
				System.out.println("Compare " + a + " to " + b);
		if (a != b) {
			failTest(testName + " - NOT EQUAL");
		} else {
			System.out.println("TEST PASSED: " + testName);
		}
	}

	private void assertEpsilonEqual (long a, long b, String testName) {
		System.out.println("Compare " + a + " to " + b);
		if (Math.abs(a - b) > oneMilliInNanos) {
			failTest(testName + " - NOT EQUAL");
		} else {
			System.out.println("TEST PASSED: " + testName);
		}
	}

}
