

package org.springframework.boot.loader.tools;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.security.DigestInputStream;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;


public abstract class FileUtils {

	
	public static void removeDuplicatesFromOutputDirectory(File outputDirectory,
			File originDirectory) {
		if (originDirectory.isDirectory()) {
			for (String name : originDirectory.list()) {
				File targetFile = new File(outputDirectory, name);
				if (targetFile.exists() && targetFile.canWrite()) {
					if (!targetFile.isDirectory()) {
						targetFile.delete();
					}
					else {
						FileUtils.removeDuplicatesFromOutputDirectory(targetFile,
								new File(originDirectory, name));
					}
				}
			}
		}
	}

	
	public static String sha1Hash(File file) throws IOException {
		try {
			try (DigestInputStream inputStream = new DigestInputStream(
					new FileInputStream(file), MessageDigest.getInstance("SHA-1"))) {
				byte[] buffer = new byte[4098];
				while (inputStream.read(buffer) != -1) {
									}
				return bytesToHex(inputStream.getMessageDigest().digest());
			}
		}
		catch (NoSuchAlgorithmException ex) {
			throw new IllegalStateException(ex);
		}
	}

	private static String bytesToHex(byte[] bytes) {
		StringBuilder hex = new StringBuilder();
		for (byte b : bytes) {
			hex.append(String.format("%02x", b));
		}
		return hex.toString();
	}

}
