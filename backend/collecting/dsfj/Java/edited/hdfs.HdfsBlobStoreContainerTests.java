

package org.elasticsearch.repositories.hdfs;

import com.carrotsearch.randomizedtesting.annotations.ThreadLeakFilters;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.fs.AbstractFileSystem;
import org.apache.hadoop.fs.FileContext;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.fs.UnsupportedFileSystemException;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.blobstore.BlobContainer;
import org.elasticsearch.common.blobstore.BlobPath;
import org.elasticsearch.common.blobstore.BlobStore;
import org.elasticsearch.common.bytes.BytesArray;
import org.elasticsearch.repositories.ESBlobStoreContainerTestCase;

import javax.security.auth.Subject;
import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.net.URI;
import java.security.AccessController;
import java.security.Principal;
import java.security.PrivilegedAction;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import java.util.Collections;

import static org.elasticsearch.repositories.ESBlobStoreTestCase.randomBytes;
import static org.elasticsearch.repositories.ESBlobStoreTestCase.readBlobFully;


@ThreadLeakFilters(filters = {HdfsClientThreadLeakFilter.class})
public class HdfsBlobStoreContainerTests extends ESBlobStoreContainerTestCase {

    @Override
    protected BlobStore newBlobStore() throws IOException {
        return new HdfsBlobStore(createTestContext(), "temp", 1024, false);
    }

    private FileContext createTestContext() {
        FileContext fileContext;
        try {
            fileContext = AccessController.doPrivileged((PrivilegedExceptionAction<FileContext>)
                () -> createContext(new URI("hdfs:        } catch (PrivilegedActionException e) {
            throw new RuntimeException(e.getCause());
        }
        return fileContext;
    }

    @SuppressForbidden(reason = "lesser of two evils (the other being a bunch of JNI/classloader nightmares)")
    private FileContext createContext(URI uri) {
                Configuration cfg = new Configuration(true);
        cfg.setClassLoader(HdfsRepository.class.getClassLoader());
        cfg.reloadConfiguration();

        Constructor<?> ctor;
        Subject subject;

        try {
            Class<?> clazz = Class.forName("org.apache.hadoop.security.User");
            ctor = clazz.getConstructor(String.class);
            ctor.setAccessible(true);
        } catch (ClassNotFoundException | NoSuchMethodException e) {
            throw new RuntimeException(e);
        }

        try {
            Principal principal = (Principal) ctor.newInstance(System.getProperty("user.name"));
            subject = new Subject(false, Collections.singleton(principal),
                    Collections.emptySet(), Collections.emptySet());
        } catch (InstantiationException | IllegalAccessException | InvocationTargetException e) {
            throw new RuntimeException(e);
        }

                cfg.setBoolean("fs.hdfs.impl.disable.cache", true);

                        cfg.set("fs.AbstractFileSystem." + uri.getScheme() + ".impl", TestingFs.class.getName());

                return Subject.doAs(subject, (PrivilegedAction<FileContext>) () -> {
            try {
                TestingFs fs = (TestingFs) AbstractFileSystem.get(uri, cfg);
                return FileContext.getFileContext(fs, cfg);
            } catch (UnsupportedFileSystemException e) {
                throw new RuntimeException(e);
            }
        });
    }

    public void testReadOnly() throws Exception {
        FileContext fileContext = createTestContext();
                HdfsBlobStore hdfsBlobStore = new HdfsBlobStore(fileContext, "dir", 1024, true);
        FileContext.Util util = fileContext.util();
        Path root = fileContext.makeQualified(new Path("dir"));
        assertFalse(util.exists(root));
        BlobPath blobPath = BlobPath.cleanPath().add("path");

                hdfsBlobStore.blobContainer(blobPath);
        Path hdfsPath = root;
        for (String p : blobPath) {
            hdfsPath = new Path(hdfsPath, p);
        }
        assertFalse(util.exists(hdfsPath));

                hdfsBlobStore = new HdfsBlobStore(fileContext, "dir", 1024, false);
        assertTrue(util.exists(root));
        BlobContainer container = hdfsBlobStore.blobContainer(blobPath);
        assertTrue(util.exists(hdfsPath));

        byte[] data = randomBytes(randomIntBetween(10, scaledRandomIntBetween(1024, 1 << 16)));
        writeBlob(container, "foo", new BytesArray(data));
        assertArrayEquals(readBlobFully(container, "foo", data.length), data);
        assertTrue(container.blobExists("foo"));
    }
}
