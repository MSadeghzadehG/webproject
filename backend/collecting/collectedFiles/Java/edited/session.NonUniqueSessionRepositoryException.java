

package org.springframework.boot.autoconfigure.session;

import java.util.Collections;
import java.util.List;

import org.springframework.session.SessionRepository;
import org.springframework.util.ObjectUtils;


public class NonUniqueSessionRepositoryException extends RuntimeException {

	private final List<Class<?>> availableCandidates;

	public NonUniqueSessionRepositoryException(List<Class<?>> availableCandidates) {
		super("Multiple session repository candidates are available, set the "
				+ "'spring.session.store-type' property accordingly");
		this.availableCandidates = (!ObjectUtils.isEmpty(availableCandidates)
				? availableCandidates : Collections.emptyList());
	}

	public List<Class<?>> getAvailableCandidates() {
		return this.availableCandidates;
	}

}
