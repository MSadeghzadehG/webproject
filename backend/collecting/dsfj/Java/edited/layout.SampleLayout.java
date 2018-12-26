

package sample.layout;

import java.io.ByteArrayInputStream;
import java.io.IOException;

import org.springframework.boot.loader.tools.CustomLoaderLayout;
import org.springframework.boot.loader.tools.Layouts;
import org.springframework.boot.loader.tools.LoaderClassesWriter;


public class SampleLayout extends Layouts.Jar implements CustomLoaderLayout {

	private String name;

	public SampleLayout(String name) {
		this.name = name;
	}

	@Override
	public void writeLoadedClasses(LoaderClassesWriter writer) throws IOException {
		writer.writeEntry(this.name, new ByteArrayInputStream("test".getBytes()));
	}

}
