
package org.elasticsearch.common;

import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.monitor.jvm.JvmInfo;

import java.io.IOException;
import java.io.OutputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;


public final class PidFile {

    private final long pid;
    private final Path path;
    private final boolean deleteOnExit;

    private PidFile(Path path, boolean deleteOnExit, long pid) throws IOException {
        this.path = path;
        this.deleteOnExit = deleteOnExit;
        this.pid = pid;
    }

    
    public static PidFile create(Path path, boolean deleteOnExit) throws IOException {
        return create(path, deleteOnExit, JvmInfo.jvmInfo().pid());
    }

    static PidFile create(Path path, boolean deleteOnExit, long pid) throws IOException {
        Path parent = path.getParent();
        if (parent != null) {
            if (Files.exists(parent) && Files.isDirectory(parent) == false) {
                throw new IllegalArgumentException(parent + " exists but is not a directory");
            }
            if (Files.exists(parent) == false) {
                                                                                Files.createDirectories(parent);
            }
        }
        if (Files.exists(path) && Files.isRegularFile(path) == false) {
            throw new IllegalArgumentException(path + " exists but is not a regular file");
        }

        try(OutputStream stream = Files.newOutputStream(path, StandardOpenOption.CREATE, StandardOpenOption.TRUNCATE_EXISTING)) {
            stream.write(Long.toString(pid).getBytes(StandardCharsets.UTF_8));
        }

        if (deleteOnExit) {
            addShutdownHook(path);
        }
        return new PidFile(path, deleteOnExit, pid);
    }


    
    public long getPid() {
        return pid;
    }

    
    public Path getPath() {
        return path;
    }

    
    public boolean isDeleteOnExit() {
        return deleteOnExit;
    }

    private static void addShutdownHook(final Path path) {
        Runtime.getRuntime().addShutdownHook(new Thread() {
            @Override
            public void run() {
                try {
                    Files.deleteIfExists(path);
                } catch (IOException e) {
                    throw new ElasticsearchException("Failed to delete pid file " + path, e);
                }
            }
        });
    }
}
