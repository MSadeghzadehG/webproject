

package org.springframework.boot.system;

import org.junit.Test;

import static org.assertj.core.api.Assertions.assertThat;


public class JavaVersionTests {

	@Test
	public void getJavaVersionShouldBeAvailable() {
		assertThat(JavaVersion.getJavaVersion()).isNotNull();
	}

	@Test
	public void compareToWhenComparingSmallerToGreaterShouldBeLessThanZero() {
		assertThat(JavaVersion.EIGHT.compareTo(JavaVersion.NINE)).isLessThan(0);
	}

	@Test
	public void compareToWhenComparingGreaterToSmallerShouldBeGreaterThanZero() {
		assertThat(JavaVersion.NINE.compareTo(JavaVersion.EIGHT)).isGreaterThan(0);
	}

	@Test
	public void compareToWhenComparingSameShouldBeZero() {
		assertThat(JavaVersion.EIGHT.compareTo(JavaVersion.EIGHT)).isEqualTo(0);
	}

	@Test
	public void isEqualOrNewerThanWhenComparingSameShouldBeTrue() {
		assertThat(JavaVersion.EIGHT.isEqualOrNewerThan(JavaVersion.EIGHT)).isTrue();
	}

	@Test
	public void isEqualOrNewerThanWhenSmallerToGreaterShouldBeFalse() {
		assertThat(JavaVersion.EIGHT.isEqualOrNewerThan(JavaVersion.NINE)).isFalse();
	}

	@Test
	public void isEqualOrNewerThanWhenGreaterToSmallerShouldBeTrue() {
		assertThat(JavaVersion.NINE.isEqualOrNewerThan(JavaVersion.EIGHT)).isTrue();
	}

	@Test
	public void isOlderThanThanWhenComparingSameShouldBeFalse() {
		assertThat(JavaVersion.EIGHT.isOlderThan(JavaVersion.EIGHT)).isFalse();
	}

	@Test
	public void isOlderThanWhenSmallerToGreaterShouldBeTrue() {
		assertThat(JavaVersion.EIGHT.isOlderThan(JavaVersion.NINE)).isTrue();
	}

	@Test
	public void isOlderThanWhenGreaterToSmallerShouldBeFalse() {
		assertThat(JavaVersion.NINE.isOlderThan(JavaVersion.EIGHT)).isFalse();
	}

}
