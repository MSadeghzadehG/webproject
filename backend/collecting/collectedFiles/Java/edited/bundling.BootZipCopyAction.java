

package org.springframework.boot.gradle.tasks.bundling;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.HashSet;
import java.util.Set;
import java.util.function.Function;
import java.util.zip.CRC32;
import java.util.zip.ZipInputStream;

import org.apache.commons.compress.archivers.zip.UnixStat;
import org.apache.commons.compress.archivers.zip.ZipArchiveEntry;
import org.apache.commons.compress.archivers.zip.ZipArchiveOutputStream;
import org.gradle.api.GradleException;
import org.gradle.api.file.FileCopyDetails;
import org.gradle.api.file.FileTreeElement;
import org.gradle.api.internal.file.CopyActionProcessingStreamAction;
import org.gradle.api.internal.file.copy.CopyAction;
import org.gradle.api.internal.file.copy.CopyActionProcessingStream;
import org.gradle.api.internal.file.copy.FileCopyDetailsInternal;
import org.gradle.api.specs.Spec;
import org.gradle.api.specs.Specs;
import org.gradle.api.tasks.WorkResult;

import org.springframework.boot.loader.tools.DefaultLaunchScript;
import org.springframework.boot.loader.tools.FileUtils;


class BootZipCopyAction implements CopyAction {

	private static final long CONSTANT_TIME_FOR_ZIP_ENTRIES = new GregorianCalendar(1980,
			Calendar.FEBRUARY, 1, 0, 0, 0).getTimeInMillis();

	private final File output;

	private final boolean preserveFileTimestamps;

	private final boolean includeDefaultLoader;

	private final Spec<FileTreeElement> requiresUnpack;

	private final Spec<FileTreeElement> exclusions;

	private final LaunchScriptConfiguration launchScript;

	private final Function<FileCopyDetails, ZipCompression> compressionResolver;

	private final String encoding;

	BootZipCopyAction(File output, boolean preserveFileTimestamps,
			boolean includeDefaultLoader, Spec<FileTreeElement> requiresUnpack,
			Spec<FileTreeElement> exclusions, LaunchScriptConfiguration launchScript,
			Function<FileCopyDetails, ZipCompression> compressionResolver,
			String encoding) {
		this.output = output;
		this.preserveFileTimestamps = preserveFileTimestamps;
		this.includeDefaultLoader = includeDefaultLoader;
		this.requiresUnpack = requiresUnpack;
		this.exclusions = exclusions;
		this.launchScript = launchScript;
		this.compressionResolver = compressionResolver;
		this.encoding = encoding;
	}

	@Override
	public WorkResult execute(CopyActionProcessingStream stream) {
		ZipArchiveOutputStream zipStream;
		Spec<FileTreeElement> loaderEntries;
		try {
			FileOutputStream fileStream = new FileOutputStream(this.output);
			writeLaunchScriptIfNecessary(fileStream);
			zipStream = new ZipArchiveOutputStream(fileStream);
			if (this.encoding != null) {
				zipStream.setEncoding(this.encoding);
			}
			loaderEntries = writeLoaderClassesIfNecessary(zipStream);
		}
		catch (IOException ex) {
			throw new GradleException("Failed to create " + this.output, ex);
		}
		try {
			stream.process(new ZipStreamAction(zipStream, this.output,
					this.preserveFileTimestamps, this.requiresUnpack,
					createExclusionSpec(loaderEntries), this.compressionResolver));
		}
		finally {
			try {
				zipStream.close();
			}
			catch (IOException ex) {
							}
		}
		return () -> true;
	}

	@SuppressWarnings("unchecked")
	private Spec<FileTreeElement> createExclusionSpec(
			Spec<FileTreeElement> loaderEntries) {
		return Specs.union(loaderEntries, this.exclusions);
	}

	private Spec<FileTreeElement> writeLoaderClassesIfNecessary(
			ZipArchiveOutputStream out) {
		if (!this.includeDefaultLoader) {
			return Specs.satisfyNone();
		}
		return writeLoaderClasses(out);
	}

	private Spec<FileTreeElement> writeLoaderClasses(ZipArchiveOutputStream out) {
		try (ZipInputStream in = new ZipInputStream(getClass()
				.getResourceAsStream("/META-INF/loader/spring-boot-loader.jar"))) {
			Set<String> entries = new HashSet<>();
			java.util.zip.ZipEntry entry;
			while ((entry = in.getNextEntry()) != null) {
				if (entry.isDirectory() && !entry.getName().startsWith("META-INF/")) {
					writeDirectory(new ZipArchiveEntry(entry), out);
					entries.add(entry.getName());
				}
				else if (entry.getName().endsWith(".class")) {
					writeClass(new ZipArchiveEntry(entry), in, out);
				}
			}
			return (element) -> {
				String path = element.getRelativePath().getPathString();
				if (element.isDirectory() && !path.endsWith(("/"))) {
					path += "/";
				}
				return entries.contains(path);
			};
		}
		catch (IOException ex) {
			throw new GradleException("Failed to write loader classes", ex);
		}
	}

	private void writeDirectory(ZipArchiveEntry entry, ZipArchiveOutputStream out)
			throws IOException {
		prepareEntry(entry, UnixStat.DIR_FLAG | UnixStat.DEFAULT_DIR_PERM);
		out.putArchiveEntry(entry);
		out.closeArchiveEntry();
	}

	private void writeClass(ZipArchiveEntry entry, ZipInputStream in,
			ZipArchiveOutputStream out) throws IOException {
		prepareEntry(entry, UnixStat.FILE_FLAG | UnixStat.DEFAULT_FILE_PERM);
		out.putArchiveEntry(entry);
		byte[] buffer = new byte[4096];
		int read;
		while ((read = in.read(buffer)) > 0) {
			out.write(buffer, 0, read);
		}
		out.closeArchiveEntry();
	}

	private void prepareEntry(ZipArchiveEntry entry, int unixMode) {
		if (!this.preserveFileTimestamps) {
			entry.setTime(CONSTANT_TIME_FOR_ZIP_ENTRIES);
		}
		entry.setUnixMode(unixMode);
	}

	private void writeLaunchScriptIfNecessary(FileOutputStream fileStream) {
		try {
			if (this.launchScript != null) {
				fileStream.write(new DefaultLaunchScript(this.launchScript.getScript(),
						this.launchScript.getProperties()).toByteArray());
				this.output.setExecutable(true);
			}
		}
		catch (IOException ex) {
			throw new GradleException("Failed to write launch script to " + this.output,
					ex);
		}
	}

	private static final class ZipStreamAction
			implements CopyActionProcessingStreamAction {

		private final ZipArchiveOutputStream zipStream;

		private final File output;

		private final boolean preserveFileTimestamps;

		private final Spec<FileTreeElement> requiresUnpack;

		private final Spec<FileTreeElement> exclusions;

		private final Function<FileCopyDetails, ZipCompression> compressionType;

		private ZipStreamAction(ZipArchiveOutputStream zipStream, File output,
				boolean preserveFileTimestamps, Spec<FileTreeElement> requiresUnpack,
				Spec<FileTreeElement> exclusions,
				Function<FileCopyDetails, ZipCompression> compressionType) {
			this.zipStream = zipStream;
			this.output = output;
			this.preserveFileTimestamps = preserveFileTimestamps;
			this.requiresUnpack = requiresUnpack;
			this.exclusions = exclusions;
			this.compressionType = compressionType;
		}

		@Override
		public void processFile(FileCopyDetailsInternal details) {
			if (this.exclusions.isSatisfiedBy(details)) {
				return;
			}
			try {
				if (details.isDirectory()) {
					createDirectory(details);
				}
				else {
					createFile(details);
				}
			}
			catch (IOException ex) {
				throw new GradleException(
						"Failed to add " + details + " to " + this.output, ex);
			}
		}

		private void createDirectory(FileCopyDetailsInternal details) throws IOException {
			ZipArchiveEntry archiveEntry = new ZipArchiveEntry(
					details.getRelativePath().getPathString() + '/');
			archiveEntry.setUnixMode(UnixStat.DIR_FLAG | details.getMode());
			archiveEntry.setTime(getTime(details));
			this.zipStream.putArchiveEntry(archiveEntry);
			this.zipStream.closeArchiveEntry();
		}

		private void createFile(FileCopyDetailsInternal details) throws IOException {
			String relativePath = details.getRelativePath().getPathString();
			ZipArchiveEntry archiveEntry = new ZipArchiveEntry(relativePath);
			archiveEntry.setUnixMode(UnixStat.FILE_FLAG | details.getMode());
			archiveEntry.setTime(getTime(details));
			ZipCompression compression = this.compressionType.apply(details);
			if (compression == ZipCompression.STORED) {
				prepareStoredEntry(details, archiveEntry);
			}
			this.zipStream.putArchiveEntry(archiveEntry);
			details.copyTo(this.zipStream);
			this.zipStream.closeArchiveEntry();
		}

		private void prepareStoredEntry(FileCopyDetailsInternal details,
				ZipArchiveEntry archiveEntry) throws IOException {
			archiveEntry.setMethod(java.util.zip.ZipEntry.STORED);
			archiveEntry.setSize(details.getSize());
			archiveEntry.setCompressedSize(details.getSize());
			Crc32OutputStream crcStream = new Crc32OutputStream();
			details.copyTo(crcStream);
			archiveEntry.setCrc(crcStream.getCrc());
			if (this.requiresUnpack.isSatisfiedBy(details)) {
				archiveEntry
						.setComment("UNPACK:" + FileUtils.sha1Hash(details.getFile()));
			}
		}

		private long getTime(FileCopyDetails details) {
			return this.preserveFileTimestamps ? details.getLastModified()
					: CONSTANT_TIME_FOR_ZIP_ENTRIES;
		}

	}

	
	private static final class Crc32OutputStream extends OutputStream {

		private final CRC32 crc32 = new CRC32();

		@Override
		public void write(int b) throws IOException {
			this.crc32.update(b);
		}

		@Override
		public void write(byte[] b) throws IOException {
			this.crc32.update(b);
		}

		@Override
		public void write(byte[] b, int off, int len) throws IOException {
			this.crc32.update(b, off, len);
		}

		private long getCrc() {
			return this.crc32.getValue();
		}

	}

}
