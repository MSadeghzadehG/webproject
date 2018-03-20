

package org.elasticsearch.common.io;

import org.apache.logging.log4j.Logger;
import org.apache.lucene.util.Constants;
import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.common.Strings;
import org.elasticsearch.common.SuppressForbidden;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.stream.StreamSupport;


public final class FileSystemUtils {

    private FileSystemUtils() {} 
    
    public static boolean exists(Path... files) {
        for (Path file : files) {
            if (Files.exists(file)) {
                return true;
            }
        }
        return false;
    }

    
    public static boolean isHidden(Path path) {
        Path fileName = path.getFileName();
        if (fileName == null) {
            return false;
        }
        return fileName.toString().startsWith(".");
    }

    
    public static boolean isDesktopServicesStore(final Path path) {
        return Constants.MAC_OS_X && Files.isRegularFile(path) && ".DS_Store".equals(path.getFileName().toString());
    }

    
    public static Path append(Path base, Path path, int strip) {
        for (Path subPath : path) {
            if (strip-- > 0) {
                continue;
            }
            base = base.resolve(subPath.toString());
        }
        return base;
    }

    
    public static void deleteSubDirectories(Path... paths) throws IOException {
        for (Path path : paths) {
            try (DirectoryStream<Path> stream = Files.newDirectoryStream(path)) {
                for (Path subPath : stream) {
                    if (Files.isDirectory(subPath)) {
                        IOUtils.rm(subPath);
                    }
                }
            }
        }
    }


    
    public static boolean isAccessibleDirectory(Path directory, Logger logger) {
        assert directory != null && logger != null;

        if (!Files.exists(directory)) {
            logger.debug("[{}] directory does not exist.", directory.toAbsolutePath());
            return false;
        }
        if (!Files.isDirectory(directory)) {
            logger.debug("[{}] should be a directory but is not.", directory.toAbsolutePath());
            return false;
        }
        if (!Files.isReadable(directory)) {
            logger.debug("[{}] directory is not readable.", directory.toAbsolutePath());
            return false;
        }
        return true;
    }

    
    @SuppressForbidden(reason = "Will only open url streams for local files")
    public static InputStream openFileURLStream(URL url) throws IOException {
        String protocol = url.getProtocol();
        if ("file".equals(protocol) == false && "jar".equals(protocol) == false) {
            throw new IllegalArgumentException("Invalid protocol [" + protocol + "], must be [file] or [jar]");
        }
        if (Strings.isEmpty(url.getHost()) == false) {
            throw new IllegalArgumentException("URL cannot have host. Found: [" + url.getHost() + ']');
        }
        if (url.getPort() != -1) {
            throw new IllegalArgumentException("URL cannot have port. Found: [" + url.getPort() + ']');
        }
        return url.openStream();
    }

    
    public static Path[] files(Path from, DirectoryStream.Filter<Path> filter) throws IOException {
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(from, filter)) {
            return toArray(stream);
        }
    }

    
    public static Path[] files(Path directory) throws IOException {
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(directory)) {
            return toArray(stream);
        }
    }

    
    public static Path[] files(Path directory, String glob) throws IOException {
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(directory, glob)) {
            return toArray(stream);
        }
    }

    private static Path[] toArray(DirectoryStream<Path> stream) {
        return StreamSupport.stream(stream.spliterator(), false).toArray(length -> new Path[length]);
    }

}
