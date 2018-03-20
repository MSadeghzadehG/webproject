

package org.elasticsearch.index.translog;

import org.elasticsearch.index.seqno.SequenceNumbers;
import org.elasticsearch.test.ESTestCase;

import java.io.IOException;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;

import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.equalTo;


public class TranslogVersionTests extends ESTestCase {

    private void checkFailsToOpen(String file, String expectedMessage) throws IOException {
        Path translogFile = getDataPath(file);
        assertThat("test file should exist", Files.exists(translogFile), equalTo(true));
        try {
            openReader(translogFile, 0);
            fail("should be able to open an old translog");
        } catch (IllegalStateException e) {
            assertThat(e.getMessage(), containsString(expectedMessage));
        }

    }

    public void testV0LegacyTranslogVersion() throws Exception {
        checkFailsToOpen("/org/elasticsearch/index/translog/translog-v0.binary", "pre-1.4 translog");
    }

    public void testV1ChecksummedTranslogVersion() throws Exception {
        checkFailsToOpen("/org/elasticsearch/index/translog/translog-v1.binary", "pre-2.0 translog");
    }

    public void testCorruptedTranslogs() throws Exception {
        try {
            Path translogFile = getDataPath("/org/elasticsearch/index/translog/translog-v1-corrupted-magic.binary");
            assertThat("test file should exist", Files.exists(translogFile), equalTo(true));
            openReader(translogFile, 0);
            fail("should have thrown an exception about the header being corrupt");
        } catch (TranslogCorruptedException e) {
            assertThat("translog corruption from header: " + e.getMessage(),
                    e.getMessage().contains("translog looks like version 1 or later, but has corrupted header"), equalTo(true));
        }

        try {
            Path translogFile = getDataPath("/org/elasticsearch/index/translog/translog-invalid-first-byte.binary");
            assertThat("test file should exist", Files.exists(translogFile), equalTo(true));
            openReader(translogFile, 0);
            fail("should have thrown an exception about the header being corrupt");
        } catch (TranslogCorruptedException e) {
            assertThat("translog corruption from header: " + e.getMessage(),
                    e.getMessage().contains("Invalid first byte in translog file, got: 1, expected 0x00 or 0x3f"), equalTo(true));
        }

        checkFailsToOpen("/org/elasticsearch/index/translog/translog-v1-corrupted-body.binary", "pre-2.0 translog");
    }

    public void testTruncatedTranslog() throws Exception {
        checkFailsToOpen("/org/elasticsearch/index/translog/translog-v1-truncated.binary", "pre-2.0 translog");
    }

    public TranslogReader openReader(final Path path, final long id) throws IOException {
        try (FileChannel channel = FileChannel.open(path, StandardOpenOption.READ)) {
            final long minSeqNo = SequenceNumbers.NO_OPS_PERFORMED;
            final long maxSeqNo = SequenceNumbers.NO_OPS_PERFORMED;
            final Checkpoint checkpoint =
                new Checkpoint(Files.size(path), 1, id, minSeqNo, maxSeqNo, SequenceNumbers.UNASSIGNED_SEQ_NO, id);
            return TranslogReader.open(channel, path, checkpoint, null);
        }
    }
}
