

package org.springframework.boot.devtools.remote.client;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.ObjectOutputStream;
import java.net.ConnectException;
import java.net.MalformedURLException;
import java.net.URI;
import java.net.URISyntaxException;
import java.net.URL;
import java.util.Collections;
import java.util.EnumMap;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.boot.devtools.classpath.ClassPathChangedEvent;
import org.springframework.boot.devtools.filewatch.ChangedFile;
import org.springframework.boot.devtools.filewatch.ChangedFiles;
import org.springframework.boot.devtools.restart.classloader.ClassLoaderFile;
import org.springframework.boot.devtools.restart.classloader.ClassLoaderFile.Kind;
import org.springframework.boot.devtools.restart.classloader.ClassLoaderFiles;
import org.springframework.context.ApplicationListener;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.client.ClientHttpRequest;
import org.springframework.http.client.ClientHttpRequestFactory;
import org.springframework.http.client.ClientHttpResponse;
import org.springframework.util.Assert;
import org.springframework.util.FileCopyUtils;


public class ClassPathChangeUploader
		implements ApplicationListener<ClassPathChangedEvent> {

	private static final Map<ChangedFile.Type, ClassLoaderFile.Kind> TYPE_MAPPINGS;

	static {
		Map<ChangedFile.Type, ClassLoaderFile.Kind> map = new EnumMap<>(
				ChangedFile.Type.class);
		map.put(ChangedFile.Type.ADD, ClassLoaderFile.Kind.ADDED);
		map.put(ChangedFile.Type.DELETE, ClassLoaderFile.Kind.DELETED);
		map.put(ChangedFile.Type.MODIFY, ClassLoaderFile.Kind.MODIFIED);
		TYPE_MAPPINGS = Collections.unmodifiableMap(map);
	}

	private static final Log logger = LogFactory.getLog(ClassPathChangeUploader.class);

	private final URI uri;

	private final ClientHttpRequestFactory requestFactory;

	public ClassPathChangeUploader(String url, ClientHttpRequestFactory requestFactory) {
		Assert.hasLength(url, "URL must not be empty");
		Assert.notNull(requestFactory, "RequestFactory must not be null");
		try {
			this.uri = new URL(url).toURI();
		}
		catch (URISyntaxException | MalformedURLException ex) {
			throw new IllegalArgumentException("Malformed URL '" + url + "'");
		}
		this.requestFactory = requestFactory;
	}

	@Override
	public void onApplicationEvent(ClassPathChangedEvent event) {
		try {
			ClassLoaderFiles classLoaderFiles = getClassLoaderFiles(event);
			byte[] bytes = serialize(classLoaderFiles);
			performUpload(classLoaderFiles, bytes);
		}
		catch (IOException ex) {
			throw new IllegalStateException(ex);
		}
	}

	private void performUpload(ClassLoaderFiles classLoaderFiles, byte[] bytes)
			throws IOException {
		try {
			while (true) {
				try {
					ClientHttpRequest request = this.requestFactory
							.createRequest(this.uri, HttpMethod.POST);
					HttpHeaders headers = request.getHeaders();
					headers.setContentType(MediaType.APPLICATION_OCTET_STREAM);
					headers.setContentLength(bytes.length);
					FileCopyUtils.copy(bytes, request.getBody());
					ClientHttpResponse response = request.execute();
					Assert.state(response.getStatusCode() == HttpStatus.OK,
							"Unexpected " + response.getStatusCode()
									+ " response uploading class files");
					logUpload(classLoaderFiles);
					return;
				}
				catch (ConnectException ex) {
					logger.warn("Failed to connect when uploading to " + this.uri
							+ ". Upload will be retried in 2 seconds");
					Thread.sleep(2000);
				}
			}
		}
		catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
			throw new IllegalStateException(ex);
		}
	}

	private void logUpload(ClassLoaderFiles classLoaderFiles) {
		int size = classLoaderFiles.size();
		logger.info(
				"Uploaded " + size + " class " + (size == 1 ? "resource" : "resources"));
	}

	private byte[] serialize(ClassLoaderFiles classLoaderFiles) throws IOException {
		ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
		ObjectOutputStream objectOutputStream = new ObjectOutputStream(outputStream);
		objectOutputStream.writeObject(classLoaderFiles);
		objectOutputStream.close();
		return outputStream.toByteArray();
	}

	private ClassLoaderFiles getClassLoaderFiles(ClassPathChangedEvent event)
			throws IOException {
		ClassLoaderFiles files = new ClassLoaderFiles();
		for (ChangedFiles changedFiles : event.getChangeSet()) {
			String sourceFolder = changedFiles.getSourceFolder().getAbsolutePath();
			for (ChangedFile changedFile : changedFiles) {
				files.addFile(sourceFolder, changedFile.getRelativeName(),
						asClassLoaderFile(changedFile));
			}
		}
		return files;
	}

	private ClassLoaderFile asClassLoaderFile(ChangedFile changedFile)
			throws IOException {
		ClassLoaderFile.Kind kind = TYPE_MAPPINGS.get(changedFile.getType());
		byte[] bytes = (kind == Kind.DELETED ? null
				: FileCopyUtils.copyToByteArray(changedFile.getFile()));
		long lastModified = (kind == Kind.DELETED ? System.currentTimeMillis()
				: changedFile.getFile().lastModified());
		return new ClassLoaderFile(kind, lastModified, bytes);
	}

}
