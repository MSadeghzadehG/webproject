
package org.elasticsearch.common.blobstore;

import org.apache.lucene.util.LuceneTestCase;
import org.elasticsearch.common.blobstore.fs.FsBlobStore;
import org.elasticsearch.common.bytes.BytesArray;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.unit.ByteSizeUnit;
import org.elasticsearch.common.unit.ByteSizeValue;
import org.elasticsearch.repositories.ESBlobStoreTestCase;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

@LuceneTestCase.SuppressFileSystems("ExtrasFS")
public class FsBlobStoreTests extends ESBlobStoreTestCase {
    protected BlobStore newBlobStore() throws IOException {
        Path tempDir = createTempDir();
        Settings settings = randomBoolean() ? Settings.EMPTY : Settings.builder().put("buffer_size", new ByteSizeValue(randomIntBetween(1, 100), ByteSizeUnit.KB)).build();
        return new FsBlobStore(settings, tempDir);
    }

    public void testReadOnly() throws Exception {
        Settings settings = Settings.builder().put("readonly", true).build();
        Path tempDir = createTempDir();
        Path path = tempDir.resolve("bar");

        try (FsBlobStore store = new FsBlobStore(settings, path)) {
            assertFalse(Files.exists(path));
            BlobPath blobPath = BlobPath.cleanPath().add("foo");
            store.blobContainer(blobPath);
            Path storePath = store.path();
            for (String d : blobPath) {
                storePath = storePath.resolve(d);
            }
            assertFalse(Files.exists(storePath));
        }

        settings = randomBoolean() ? Settings.EMPTY : Settings.builder().put("readonly", false).build();
        try (FsBlobStore store = new FsBlobStore(settings, path)) {
            assertTrue(Files.exists(path));
            BlobPath blobPath = BlobPath.cleanPath().add("foo");
            BlobContainer container = store.blobContainer(blobPath);
            Path storePath = store.path();
            for (String d : blobPath) {
                storePath = storePath.resolve(d);
            }
            assertTrue(Files.exists(storePath));
            assertTrue(Files.isDirectory(storePath));

            byte[] data = randomBytes(randomIntBetween(10, scaledRandomIntBetween(1024, 1 << 16)));
            writeBlob(container, "test", new BytesArray(data));
            assertArrayEquals(readBlobFully(container, "test", data.length), data);
            assertTrue(container.blobExists("test"));
        }
    }
}
