

package org.springframework.boot.test.autoconfigure.restdocs;

import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;

import org.assertj.core.api.Condition;
import org.assertj.core.description.TextDescription;

import org.springframework.util.FileCopyUtils;


class ContentContainingCondition extends Condition<File> {

	private final String toContain;

	ContentContainingCondition(String toContain) {
		super(new TextDescription("content containing %s", toContain));
		this.toContain = toContain;
	}

	@Override
	public boolean matches(File value) {
		try (Reader reader = new FileReader(value)) {
			String content = FileCopyUtils.copyToString(reader);
			return content.contains(this.toContain);
		}
		catch (IOException ex) {
			throw new IllegalStateException(ex);
		}
	}

}
