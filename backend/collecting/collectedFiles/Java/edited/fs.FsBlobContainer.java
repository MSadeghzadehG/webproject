

package org.elasticsearch.common.blobstore.fs;

import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.common.blobstore.BlobMetaData;
import org.elasticsearch.common.blobstore.BlobPath;
import org.elasticsearch.common.blobstore.support.AbstractBlobContainer;
import org.elasticsearch.common.blobstore.support.PlainBlobMetaData;
import org.elasticsearch.common.io.Streams;

import java.io.BufferedInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.file.DirectoryStream;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.FileVisitResult;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;
import java.nio.file.Path;
import java.nio.file.SimpleFileVisitor;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.nio.file.attribute.BasicFileAttributes;
import java.util.HashMap;
import java.util.Map;

import static java.util.Collections.unmodifiableMap;


public class FsBlobContainer extends AbstractBlobContainer {

    protected final FsBlobStore blobStore;

    protected final Path path;

    public FsBlobContainer(FsBlobStore blobStore, BlobPath blobPath, Path path) {
        super(blobPath);
        this.blobStore = blobStore;
        this.path = path;
    }

    @Override
    public Map<String, BlobMetaData> listBlobs() throws IOException {
        return listBlobsByPrefix(null);
    }

    @Override
    public Map<String, BlobMetaData> listBlobsByPrefix(String blobNamePrefix) throws IOException {
                Map<String, BlobMetaData> builder = new HashMap<>();

        blobNamePrefix = blobNamePrefix == null ? "" : blobNamePrefix;
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(path, blobNamePrefix + "*")) {
            for (Path file : stream) {
                final BasicFileAttributes attrs = Files.readAttributes(file, BasicFileAttributes.class);
                if (attrs.isRegularFile()) {
                    builder.put(file.getFileName().toString(), new PlainBlobMetaData(file.getFileName().toString(), attrs.size()));
                }
            }
        }
        return unmodifiableMap(builder);
    }

    @Override
    public void deleteBlob(String blobName) throws IOException {
        Path blobPath = path.resolve(blobName);
        if (Files.isDirectory(blobPath)) {
                                    Files.walkFileTree(blobPath, new SimpleFileVisitor<Path>() {
                @Override
                public FileVisitResult postVisitDirectory(Path dir, IOException exc) throws IOException {
                    Files.delete(dir);
                    return FileVisitResult.CONTINUE;
                }
            });
        } else {
            Files.delete(blobPath);
        }
    }

    @Override
    public boolean blobExists(String blobName) {
        return Files.exists(path.resolve(blobName));
    }

    @Override
    public InputStream readBlob(String name) throws IOException {
        final Path resolvedPath = path.resolve(name);
        try {
            return new BufferedInputStream(Files.newInputStream(resolvedPath), blobStore.bufferSizeInBytes());
        } catch (FileNotFoundException fnfe) {
            throw new NoSuchFileException("[" + name + "] blob not found");
        }
    }

    @Override
    public void writeBlob(String blobName, InputStream inputStream, long blobSize) throws IOException {
        if (blobExists(blobName)) {
            throw new FileAlreadyExistsException("blob [" + blobName + "] already exists, cannot overwrite");
        }
        final Path file = path.resolve(blobName);
        try (OutputStream outputStream = Files.newOutputStream(file, StandardOpenOption.CREATE_NEW)) {
            Streams.copy(inputStream, outputStream, new byte[blobStore.bufferSizeInBytes()]);
        }
        IOUtils.fsync(file, false);
        IOUtils.fsync(path, true);
    }

    @Override
    public void move(String source, String target) throws IOException {
        Path sourcePath = path.resolve(source);
        Path targetPath = path.resolve(target);
                        if (Files.exists(targetPath)) {
            throw new FileAlreadyExistsException("blob [" + targetPath + "] already exists, cannot overwrite");
        }
        Files.move(sourcePath, targetPath, StandardCopyOption.ATOMIC_MOVE);
        IOUtils.fsync(path, true);
    }
}
