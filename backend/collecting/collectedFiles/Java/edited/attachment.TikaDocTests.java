package org.elasticsearch.ingest.attachment;



import org.apache.lucene.util.LuceneTestCase.SuppressFileSystems;
import org.apache.lucene.util.TestUtil;
import org.apache.tika.metadata.Metadata;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.test.ESTestCase;

import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;


@SuppressFileSystems("ExtrasFS") public class TikaDocTests extends ESTestCase {

    
    static final String TIKA_FILES = "/org/elasticsearch/ingest/attachment/test/tika-files/";

    public void testFiles() throws Exception {
        Path tmp = createTempDir();
        logger.debug("unzipping all tika sample files");
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(PathUtils.get(getClass().getResource(TIKA_FILES).toURI()))) {
            for (Path doc : stream) {
                String filename = doc.getFileName().toString();
                TestUtil.unzip(getClass().getResourceAsStream(TIKA_FILES + filename), tmp);
            }
        }

        try (DirectoryStream<Path> stream = Files.newDirectoryStream(tmp)) {
            for (Path doc : stream) {
              logger.debug("parsing: {}", doc);
              assertParseable(doc);
            }
        }
    }

    void assertParseable(Path fileName) throws Exception {
        try {
            byte bytes[] = Files.readAllBytes(fileName);
            String parsedContent = TikaImpl.parse(bytes, new Metadata(), -1);
            assertNotNull(parsedContent);
            assertFalse(parsedContent.isEmpty());
            logger.debug("extracted content: {}", parsedContent);
        } catch (Exception e) {
            throw new RuntimeException("parsing of filename: " + fileName.getFileName() + " failed", e);
        }
    }
}
