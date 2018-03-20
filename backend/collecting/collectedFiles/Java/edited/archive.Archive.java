

package org.springframework.boot.loader.archive;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.util.List;
import java.util.jar.Manifest;

import org.springframework.boot.loader.Launcher;


public interface Archive extends Iterable<Archive.Entry> {

	
	URL getUrl() throws MalformedURLException;

	
	Manifest getManifest() throws IOException;

	
	List<Archive> getNestedArchives(EntryFilter filter) throws IOException;

	
	interface Entry {

		
		boolean isDirectory();

		
		String getName();

	}

	
	interface EntryFilter {

		
		boolean matches(Entry entry);

	}

}
