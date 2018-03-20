

package sample.layout;

import java.io.File;
import java.util.Enumeration;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

public final class Verify {

	private Verify() {
	}

	public static void verify(File file, String entry) throws Exception {
		try (ZipFile zipFile = new ZipFile(file)) {
			Enumeration<? extends ZipEntry> entries = zipFile.entries();
			while (entries.hasMoreElements()) {
				if (entries.nextElement().getName().equals(entry)) {
					return;
				}
			}
			throw new AssertionError("No entry " + entry);
		}
	}

}
