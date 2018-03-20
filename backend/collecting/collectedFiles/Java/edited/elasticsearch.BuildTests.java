

package org.elasticsearch;

import org.elasticsearch.common.io.FileSystemUtils;
import org.elasticsearch.test.ESTestCase;

import java.io.IOException;
import java.io.InputStream;
import java.net.URL;

public class BuildTests extends ESTestCase {

    
    public void testJarMetadata() throws IOException {
        URL url = Build.getElasticsearchCodeSourceLocation();
                try (InputStream ignored = FileSystemUtils.openFileURLStream(url)) {}
                assertNotNull(Build.CURRENT.date());
        assertNotNull(Build.CURRENT.shortHash());
    }

    public void testEqualsAndHashCode() {
        Build build = Build.CURRENT;
        Build another = new Build(build.shortHash(), build.date(), build.isSnapshot());
        assertEquals(build, another);
        assertEquals(build.hashCode(), another.hashCode());

        Build differentHash = new Build(randomAlphaOfLengthBetween(3, 10), build.date(), build.isSnapshot());
        assertNotEquals(build, differentHash);

        Build differentDate = new Build(build.shortHash(), "1970-01-01", build.isSnapshot());
        assertNotEquals(build, differentDate);

        Build differentSnapshot = new Build(build.shortHash(), build.date(), !build.isSnapshot());
        assertNotEquals(build, differentSnapshot);
    }
}
