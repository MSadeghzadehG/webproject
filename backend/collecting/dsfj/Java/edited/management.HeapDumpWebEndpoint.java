

package org.springframework.boot.actuate.management;

import java.io.Closeable;
import java.io.File;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.lang.management.ManagementFactory;
import java.lang.management.PlatformManagedObject;
import java.lang.reflect.Method;
import java.nio.ByteBuffer;
import java.nio.channels.ReadableByteChannel;
import java.nio.file.Files;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import org.springframework.boot.actuate.endpoint.annotation.Endpoint;
import org.springframework.boot.actuate.endpoint.annotation.ReadOperation;
import org.springframework.boot.actuate.endpoint.web.WebEndpointResponse;
import org.springframework.boot.actuate.endpoint.web.annotation.WebEndpoint;
import org.springframework.core.io.FileSystemResource;
import org.springframework.core.io.Resource;
import org.springframework.lang.Nullable;
import org.springframework.util.ClassUtils;
import org.springframework.util.ReflectionUtils;


@WebEndpoint(id = "heapdump")
public class HeapDumpWebEndpoint {

	private final long timeout;

	private final Lock lock = new ReentrantLock();

	private HeapDumper heapDumper;

	public HeapDumpWebEndpoint() {
		this(TimeUnit.SECONDS.toMillis(10));
	}

	protected HeapDumpWebEndpoint(long timeout) {
		this.timeout = timeout;
	}

	@ReadOperation
	public WebEndpointResponse<Resource> heapDump(@Nullable Boolean live) {
		try {
			if (this.lock.tryLock(this.timeout, TimeUnit.MILLISECONDS)) {
				try {
					return new WebEndpointResponse<>(
							dumpHeap(live == null ? true : live));
				}
				finally {
					this.lock.unlock();
				}
			}
		}
		catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}
		catch (IOException ex) {
			return new WebEndpointResponse<>(
					WebEndpointResponse.STATUS_INTERNAL_SERVER_ERROR);
		}
		catch (HeapDumperUnavailableException ex) {
			return new WebEndpointResponse<>(
					WebEndpointResponse.STATUS_SERVICE_UNAVAILABLE);
		}
		return new WebEndpointResponse<>(WebEndpointResponse.STATUS_TOO_MANY_REQUESTS);
	}

	private Resource dumpHeap(boolean live) throws IOException, InterruptedException {
		if (this.heapDumper == null) {
			this.heapDumper = createHeapDumper();
		}
		File file = createTempFile(live);
		this.heapDumper.dumpHeap(file, live);
		return new TemporaryFileSystemResource(file);
	}

	private File createTempFile(boolean live) throws IOException {
		String date = new SimpleDateFormat("yyyy-MM-dd-HH-mm").format(new Date());
		File file = File.createTempFile("heapdump" + date + (live ? "-live" : ""),
				".hprof");
		file.delete();
		return file;
	}

	
	protected HeapDumper createHeapDumper() throws HeapDumperUnavailableException {
		return new HotSpotDiagnosticMXBeanHeapDumper();
	}

	
	@FunctionalInterface
	protected interface HeapDumper {

		
		void dumpHeap(File file, boolean live) throws IOException, InterruptedException;

	}

	
	protected static class HotSpotDiagnosticMXBeanHeapDumper implements HeapDumper {

		private Object diagnosticMXBean;

		private Method dumpHeapMethod;

		@SuppressWarnings("unchecked")
		protected HotSpotDiagnosticMXBeanHeapDumper() {
			try {
				Class<?> diagnosticMXBeanClass = ClassUtils.resolveClassName(
						"com.sun.management.HotSpotDiagnosticMXBean", null);
				this.diagnosticMXBean = ManagementFactory.getPlatformMXBean(
						(Class<PlatformManagedObject>) diagnosticMXBeanClass);
				this.dumpHeapMethod = ReflectionUtils.findMethod(diagnosticMXBeanClass,
						"dumpHeap", String.class, Boolean.TYPE);
			}
			catch (Throwable ex) {
				throw new HeapDumperUnavailableException(
						"Unable to locate HotSpotDiagnosticMXBean", ex);
			}
		}

		@Override
		public void dumpHeap(File file, boolean live) {
			ReflectionUtils.invokeMethod(this.dumpHeapMethod, this.diagnosticMXBean,
					file.getAbsolutePath(), live);
		}

	}

	
	protected static class HeapDumperUnavailableException extends RuntimeException {

		public HeapDumperUnavailableException(String message, Throwable cause) {
			super(message, cause);
		}

	}

	private static final class TemporaryFileSystemResource extends FileSystemResource {

		private final Log logger = LogFactory.getLog(getClass());

		private TemporaryFileSystemResource(File file) {
			super(file);
		}

		@Override
		public ReadableByteChannel readableChannel() throws IOException {
			ReadableByteChannel readableChannel = super.readableChannel();
			return new ReadableByteChannel() {

				@Override
				public boolean isOpen() {
					return readableChannel.isOpen();
				}

				@Override
				public void close() throws IOException {
					closeThenDeleteFile(readableChannel);
				}

				@Override
				public int read(ByteBuffer dst) throws IOException {
					return readableChannel.read(dst);
				}

			};
		}

		@Override
		public InputStream getInputStream() throws IOException {
			return new FilterInputStream(super.getInputStream()) {

				@Override
				public void close() throws IOException {
					closeThenDeleteFile(this.in);
				}

			};
		}

		private void closeThenDeleteFile(Closeable closeable) throws IOException {
			try {
				closeable.close();
			}
			finally {
				deleteFile();
			}
		}

		private void deleteFile() {
			try {
				Files.delete(getFile().toPath());
			}
			catch (IOException ex) {
				TemporaryFileSystemResource.this.logger.warn(
						"Failed to delete temporary heap dump file '" + getFile() + "'",
						ex);
			}
		}

		@Override
		public boolean isFile() {
						return false;
		}

	}

}
