

package org.springframework.boot.test.autoconfigure.web.servlet.mockmvc;

import java.util.UUID;

import org.springframework.core.convert.converter.GenericConverter;


public class ExampleId {

	private final UUID id;

	ExampleId(UUID id) {
		this.id = id;
	}

	public UUID getId() {
		return this.id;
	}

}
