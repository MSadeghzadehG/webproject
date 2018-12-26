

package org.springframework.boot.system;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.nio.file.Files;
import java.nio.file.attribute.PosixFilePermission;
import java.util.Set;

import org.springframework.util.Assert;
import org.springframework.util.ObjectUtils;


public class ApplicationPid {

	private static final PosixFilePermission[] WRITE_PERMISSIONS = {
			PosixFilePermission.OWNER_WRITE, PosixFilePermission.GROUP_WRITE,
			PosixFilePermission.OTHERS_WRITE };

	private final String pid;

	public ApplicationPid() {
		this.pid = getPid();
	}

	protected ApplicationPid(String pid) {
		this.pid = pid;
	}

	private String getPid() {
		try {
			String jvmName = ManagementFactory.getRuntimeMXBean().getName();
			return jvmName.split("@")[0];
		}
		catch (Throwable ex) {
			return null;
		}
	}

	@Override
	public String toString() {
		return (this.pid == null ? "???" : this.pid);
	}

	@Override
	public int hashCode() {
		return ObjectUtils.nullSafeHashCode(this.pid);
	}

	@Override
	public boolean equals(Object obj) {
		if (obj == this) {
			return true;
		}
		if (obj != null && obj instanceof ApplicationPid) {
			return ObjectUtils.nullSafeEquals(this.pid, ((ApplicationPid) obj).pid);
		}
		return false;
	}

	
	public void write(File file) throws IOException {
		Assert.state(this.pid != null, "No PID available");
		createParentFolder(file);
		if (file.exists()) {
			assertCanOverwrite(file);
		}
		try (FileWriter writer = new FileWriter(file)) {
			writer.append(this.pid);
		}
	}

	private void createParentFolder(File file) {
		File parent = file.getParentFile();
		if (parent != null) {
			parent.mkdirs();
		}
	}

	private void assertCanOverwrite(File file) throws IOException {
		if (!file.canWrite() || !canWritePosixFile(file)) {
			throw new FileNotFoundException(file.toString() + " (permission denied)");
		}
	}

	private boolean canWritePosixFile(File file) throws IOException {
		try {
			Set<PosixFilePermission> permissions = Files
					.getPosixFilePermissions(file.toPath());
			for (PosixFilePermission permission : WRITE_PERMISSIONS) {
				if (permissions.contains(permission)) {
					return true;
				}
			}
			return false;
		}
		catch (UnsupportedOperationException ex) {
						return true;
		}
	}

}
