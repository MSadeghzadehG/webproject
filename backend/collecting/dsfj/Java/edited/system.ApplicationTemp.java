

package org.springframework.boot.system;

import java.io.File;
import java.security.MessageDigest;

import org.springframework.util.Assert;
import org.springframework.util.StringUtils;


public class ApplicationTemp {

	private static final char[] HEX_CHARS = "0123456789ABCDEF".toCharArray();

	private final Class<?> sourceClass;

	private volatile File dir;

	
	public ApplicationTemp() {
		this(null);
	}

	
	public ApplicationTemp(Class<?> sourceClass) {
		this.sourceClass = sourceClass;
	}

	@Override
	public String toString() {
		return getDir().getAbsolutePath();
	}

	
	public File getDir(String subDir) {
		File dir = new File(getDir(), subDir);
		dir.mkdirs();
		return dir;
	}

	
	public File getDir() {
		if (this.dir == null) {
			synchronized (this) {
				byte[] hash = generateHash(this.sourceClass);
				this.dir = new File(getTempDirectory(), toHexString(hash));
				this.dir.mkdirs();
				Assert.state(this.dir.exists(),
						() -> "Unable to create temp directory " + this.dir);
			}
		}
		return this.dir;
	}

	private File getTempDirectory() {
		String property = System.getProperty("java.io.tmpdir");
		Assert.state(StringUtils.hasLength(property), "No 'java.io.tmpdir' property set");
		File file = new File(property);
		Assert.state(file.exists(), () -> "Temp directory " + file + " does not exist");
		Assert.state(file.isDirectory(),
				() -> "Temp location " + file + " is not a directory");
		return file;
	}

	private byte[] generateHash(Class<?> sourceClass) {
		ApplicationHome home = new ApplicationHome(sourceClass);
		MessageDigest digest;
		try {
			digest = MessageDigest.getInstance("SHA-1");
			update(digest, home.getSource());
			update(digest, home.getDir());
			update(digest, System.getProperty("user.dir"));
			update(digest, System.getProperty("java.home"));
			update(digest, System.getProperty("java.class.path"));
			update(digest, System.getProperty("sun.java.command"));
			update(digest, System.getProperty("sun.boot.class.path"));
			return digest.digest();
		}
		catch (Exception ex) {
			throw new IllegalStateException(ex);
		}
	}

	private void update(MessageDigest digest, Object source) {
		if (source != null) {
			digest.update(getUpdateSourceBytes(source));
		}
	}

	private byte[] getUpdateSourceBytes(Object source) {
		if (source instanceof File) {
			return getUpdateSourceBytes(((File) source).getAbsolutePath());
		}
		return source.toString().getBytes();
	}

	private String toHexString(byte[] bytes) {
		char[] hex = new char[bytes.length * 2];
		for (int i = 0; i < bytes.length; i++) {
			int b = bytes[i] & 0xFF;
			hex[i * 2] = HEX_CHARS[b >>> 4];
			hex[i * 2 + 1] = HEX_CHARS[b & 0x0F];
		}
		return new String(hex);
	}

}
