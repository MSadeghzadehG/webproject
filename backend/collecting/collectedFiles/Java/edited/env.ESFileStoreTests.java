
package org.elasticsearch.env;

import org.elasticsearch.test.ESTestCase;

import java.io.IOException;
import java.nio.file.FileStore;
import java.nio.file.attribute.FileAttributeView;
import java.nio.file.attribute.FileStoreAttributeView;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

public class ESFileStoreTests extends ESTestCase {
    public void testNegativeSpace() throws Exception {
        FileStore mocked = mock(FileStore.class);
        when(mocked.getUsableSpace()).thenReturn(-1L);
        when(mocked.getTotalSpace()).thenReturn(-1L);
        when(mocked.getUnallocatedSpace()).thenReturn(-1L);
        assertEquals(-1, mocked.getUsableSpace());
        FileStore store = new ESFileStore(mocked);
        assertEquals(Long.MAX_VALUE, store.getUsableSpace());
        assertEquals(Long.MAX_VALUE, store.getTotalSpace());
        assertEquals(Long.MAX_VALUE, store.getUnallocatedSpace());
    }
}
