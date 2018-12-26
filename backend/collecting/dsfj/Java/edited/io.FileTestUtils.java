

package org.elasticsearch.common.io;

import org.elasticsearch.common.Nullable;
import org.junit.Assert;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;

import static org.elasticsearch.test.hamcrest.ElasticsearchAssertions.assertFileExists;
import static org.hamcrest.CoreMatchers.equalTo;
import static org.hamcrest.CoreMatchers.is;


public class FileTestUtils {

    
    public static void assertFileContent(Path dir, String filename, String expected) throws IOException {
        Assert.assertThat(Files.exists(dir), is(true));
        Path file = dir.resolve(filename);
        if (expected == null) {
            Assert.assertThat("file [" + file + "] should not exist.", Files.exists(file), is(false));
        } else {
            assertFileExists(file);
            String fileContent = new String(Files.readAllBytes(file), java.nio.charset.StandardCharsets.UTF_8);
                        Assert.assertThat(fileContent.trim(), equalTo(expected.trim()));
        }
    }

    
    public static void unzip(final Path zip, final Path destDir, @Nullable final String prefixToRemove) throws IOException {
        if (Files.notExists(zip)) {
            throw new IOException("[" + zip + "] zip file must exist");
        }
        Files.createDirectories(destDir);

        try (ZipInputStream zipInput = new ZipInputStream(Files.newInputStream(zip))) {
            ZipEntry entry;
            while ((entry = zipInput.getNextEntry()) != null) {
                final String entryPath;
                if (prefixToRemove != null) {
                    if (entry.getName().startsWith(prefixToRemove)) {
                        entryPath = entry.getName().substring(prefixToRemove.length());
                    } else {
                        throw new IOException("prefix not found: " + prefixToRemove);
                    }
                } else {
                    entryPath = entry.getName();
                }
                final Path path = Paths.get(destDir.toString(), entryPath);
                if (entry.isDirectory()) {
                    Files.createDirectories(path);
                } else {
                    Files.copy(zipInput, path);
                }
                zipInput.closeEntry();
            }
        }
    }
}
