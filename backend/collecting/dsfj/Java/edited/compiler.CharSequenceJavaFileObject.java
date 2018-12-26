package jadx.tests.api.compiler;

import javax.tools.SimpleJavaFileObject;
import java.net.URI;

public class CharSequenceJavaFileObject extends SimpleJavaFileObject {

	private CharSequence content;

	public CharSequenceJavaFileObject(String className, CharSequence content) {
		super(URI.create("string:		this.content = content;
	}

	@Override
	public CharSequence getCharContent(boolean ignoreEncodingErrors) {
		return content;
	}
}
