

package org.springframework.boot.loader.jar;

import java.io.File;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import org.springframework.boot.loader.TestJarCreator;
import org.springframework.boot.loader.data.RandomAccessData;
import org.springframework.boot.loader.data.RandomAccessDataFile;

import static org.assertj.core.api.Assertions.assertThat;


public class CentralDirectoryParserTests {

	@Rule
	public TemporaryFolder temporaryFolder = new TemporaryFolder();

	private File jarFile;

	private RandomAccessData jarData;

	@Before
	public void setup() throws Exception {
		this.jarFile = this.temporaryFolder.newFile();
		TestJarCreator.createTestJar(this.jarFile);
		this.jarData = new RandomAccessDataFile(this.jarFile);
	}

	@Test
	public void visitsInOrder() throws Exception {
		MockCentralDirectoryVisitor visitor = new MockCentralDirectoryVisitor();
		CentralDirectoryParser parser = new CentralDirectoryParser();
		parser.addVisitor(visitor);
		parser.parse(this.jarData, false);
		List<String> invocations = visitor.getInvocations();
		assertThat(invocations).startsWith("visitStart").endsWith("visitEnd")
				.contains("visitFileHeader");
	}

	@Test
	public void visitRecords() throws Exception {
		Collector collector = new Collector();
		CentralDirectoryParser parser = new CentralDirectoryParser();
		parser.addVisitor(collector);
		parser.parse(this.jarData, false);
		Iterator<CentralDirectoryFileHeader> headers = collector.getHeaders().iterator();
		assertThat(headers.next().getName().toString()).isEqualTo("META-INF/");
		assertThat(headers.next().getName().toString()).isEqualTo("META-INF/MANIFEST.MF");
		assertThat(headers.next().getName().toString()).isEqualTo("1.dat");
		assertThat(headers.next().getName().toString()).isEqualTo("2.dat");
		assertThat(headers.next().getName().toString()).isEqualTo("d/");
		assertThat(headers.next().getName().toString()).isEqualTo("d/9.dat");
		assertThat(headers.next().getName().toString()).isEqualTo("special/");
		assertThat(headers.next().getName().toString()).isEqualTo("special/\u00EB.dat");
		assertThat(headers.next().getName().toString()).isEqualTo("nested.jar");
		assertThat(headers.next().getName().toString()).isEqualTo("another-nested.jar");
		assertThat(headers.hasNext()).isFalse();
	}

	private static class Collector implements CentralDirectoryVisitor {

		private List<CentralDirectoryFileHeader> headers = new ArrayList<>();

		@Override
		public void visitStart(CentralDirectoryEndRecord endRecord,
				RandomAccessData centralDirectoryData) {
		}

		@Override
		public void visitFileHeader(CentralDirectoryFileHeader fileHeader,
				int dataOffset) {
			this.headers.add(fileHeader.clone());
		}

		@Override
		public void visitEnd() {
		}

		public List<CentralDirectoryFileHeader> getHeaders() {
			return this.headers;
		}

	}

	private static class MockCentralDirectoryVisitor implements CentralDirectoryVisitor {

		private final List<String> invocations = new ArrayList<>();

		@Override
		public void visitStart(CentralDirectoryEndRecord endRecord,
				RandomAccessData centralDirectoryData) {
			this.invocations.add("visitStart");
		}

		@Override
		public void visitFileHeader(CentralDirectoryFileHeader fileHeader,
				int dataOffset) {
			this.invocations.add("visitFileHeader");
		}

		@Override
		public void visitEnd() {
			this.invocations.add("visitEnd");
		}

		public List<String> getInvocations() {
			return this.invocations;
		}

	}

}
