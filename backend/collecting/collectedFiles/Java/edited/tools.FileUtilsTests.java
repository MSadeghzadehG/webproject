

package org.springframework.boot.loader.tools;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import org.springframework.util.FileSystemUtils;

import static org.assertj.core.api.Assertions.assertThat;


public class FileUtilsTests {

	@Rule
	public TemporaryFolder temporaryFolder = new TemporaryFolder();

	private File outputDirectory;

	private File originDirectory;

	@Before
	public void init() {
		this.outputDirectory = new File("target/test/remove");
		this.originDirectory = new File("target/test/keep");
		FileSystemUtils.deleteRecursively(this.outputDirectory);
		FileSystemUtils.deleteRecursively(this.originDirectory);
		this.outputDirectory.mkdirs();
		this.originDirectory.mkdirs();
	}

	@Test
	public void simpleDuplicateFile() throws IOException {
		File file = new File(this.outputDirectory, "logback.xml");
		file.createNewFile();
		new File(this.originDirectory, "logback.xml").createNewFile();
		FileUtils.removeDuplicatesFromOutputDirectory(this.outputDirectory,
				this.originDirectory);
		assertThat(file.exists()).isFalse();
	}

	@Test
	public void nestedDuplicateFile() throws IOException {
		assertThat(new File(this.outputDirectory, "sub").mkdirs()).isTrue();
		assertThat(new File(this.originDirectory, "sub").mkdirs()).isTrue();
		File file = new File(this.outputDirectory, "sub/logback.xml");
		file.createNewFile();
		new File(this.originDirectory, "sub/logback.xml").createNewFile();
		FileUtils.removeDuplicatesFromOutputDirectory(this.outputDirectory,
				this.originDirectory);
		assertThat(file.exists()).isFalse();
	}

	@Test
	public void nestedNonDuplicateFile() throws IOException {
		assertThat(new File(this.outputDirectory, "sub").mkdirs()).isTrue();
		assertThat(new File(this.originDirectory, "sub").mkdirs()).isTrue();
		File file = new File(this.outputDirectory, "sub/logback.xml");
		file.createNewFile();
		new File(this.originDirectory, "sub/different.xml").createNewFile();
		FileUtils.removeDuplicatesFromOutputDirectory(this.outputDirectory,
				this.originDirectory);
		assertThat(file.exists()).isTrue();
	}

	@Test
	public void nonDuplicateFile() throws IOException {
		File file = new File(this.outputDirectory, "logback.xml");
		file.createNewFile();
		new File(this.originDirectory, "different.xml").createNewFile();
		FileUtils.removeDuplicatesFromOutputDirectory(this.outputDirectory,
				this.originDirectory);
		assertThat(file.exists()).isTrue();
	}

	@Test
	public void hash() throws Exception {
		File file = this.temporaryFolder.newFile();
		try (OutputStream outputStream = new FileOutputStream(file)) {
			outputStream.write(new byte[] { 1, 2, 3 });
		}
		assertThat(FileUtils.sha1Hash(file))
				.isEqualTo("7037807198c22a7d2b0807371d763779a84fdfcf");
	}

}
