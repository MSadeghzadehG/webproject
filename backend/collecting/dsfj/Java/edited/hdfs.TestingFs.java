

package org.elasticsearch.repositories.hdfs;

import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.DelegateToFileSystem;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.RawLocalFileSystem;
import org.apache.hadoop.fs.permission.FsPermission;
import org.apache.lucene.util.LuceneTestCase;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.file.Path;
import java.nio.file.attribute.BasicFileAttributes;
import java.nio.file.spi.FileSystemProvider;
import java.nio.file.Files;
import java.nio.file.NoSuchFileException;


public class TestingFs extends DelegateToFileSystem {

        static RawLocalFileSystem wrap(final Path base) {
        final FileSystemProvider baseProvider = base.getFileSystem().provider();
        return new RawLocalFileSystem() {

            private org.apache.hadoop.fs.Path box(Path path) {
                return new org.apache.hadoop.fs.Path(path.toUri());
            }

            private Path unbox(org.apache.hadoop.fs.Path path) {
                return baseProvider.getPath(path.toUri());
            }

            @Override
            protected org.apache.hadoop.fs.Path getInitialWorkingDirectory() {
                return box(base);
            }

            @Override
            public void setPermission(org.apache.hadoop.fs.Path path, FsPermission permission) {
                           }

                        
            @Override
            public boolean supportsSymlinks() {
                return false;
            }

            @Override
            public FileStatus getFileLinkStatus(org.apache.hadoop.fs.Path path) throws IOException {
                return getFileStatus(path);
            }

            @Override
            public org.apache.hadoop.fs.Path getLinkTarget(org.apache.hadoop.fs.Path path) throws IOException {
                return path;
            }

            @Override
            public FileStatus getFileStatus(org.apache.hadoop.fs.Path path) throws IOException {
                BasicFileAttributes attributes;
                try {
                    attributes = Files.readAttributes(unbox(path), BasicFileAttributes.class);
                } catch (NoSuchFileException e) {
                                        FileNotFoundException fnfe = new FileNotFoundException("File " + path + " does not exist");
                    fnfe.initCause(e);
                    throw fnfe;
                }

                                long length = attributes.size();
                boolean isDir = attributes.isDirectory();
                int blockReplication = 1;
                long blockSize = getDefaultBlockSize(path);
                long modificationTime = attributes.creationTime().toMillis();
                return new FileStatus(length, isDir, blockReplication, blockSize, modificationTime, path);
            }
        };
    }

    public TestingFs(URI uri, Configuration configuration) throws URISyntaxException, IOException {
        super(URI.create("file:    }

    @Override
    public void checkPath(org.apache.hadoop.fs.Path path) {
          }
}
