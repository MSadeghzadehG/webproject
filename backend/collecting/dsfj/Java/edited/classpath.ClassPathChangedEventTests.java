

package org.springframework.boot.devtools.classpath;

import java.util.LinkedHashSet;
import java.util.Set;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;

import org.springframework.boot.devtools.filewatch.ChangedFiles;

import static org.assertj.core.api.Assertions.assertThat;


public class ClassPathChangedEventTests {

	@Rule
	public ExpectedException thrown = ExpectedException.none();

	private Object source = new Object();

	@Test
	public void changeSetMustNotBeNull() {
		this.thrown.expect(IllegalArgumentException.class);
		this.thrown.expectMessage("ChangeSet must not be null");
		new ClassPathChangedEvent(this.source, null, false);
	}

	@Test
	public void getChangeSet() {
		Set<ChangedFiles> changeSet = new LinkedHashSet<>();
		ClassPathChangedEvent event = new ClassPathChangedEvent(this.source, changeSet,
				false);
		assertThat(event.getChangeSet()).isSameAs(changeSet);
	}

	@Test
	public void getRestartRequired() {
		Set<ChangedFiles> changeSet = new LinkedHashSet<>();
		ClassPathChangedEvent event;
		event = new ClassPathChangedEvent(this.source, changeSet, false);
		assertThat(event.isRestartRequired()).isFalse();
		event = new ClassPathChangedEvent(this.source, changeSet, true);
		assertThat(event.isRestartRequired()).isTrue();
	}

}
