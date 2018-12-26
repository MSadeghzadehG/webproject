
package org.elasticsearch.test;

import com.carrotsearch.randomizedtesting.generators.RandomPicks;
import org.apache.logging.log4j.Logger;
import org.apache.lucene.codecs.CodecUtil;
import org.apache.lucene.store.ChecksumIndexInput;
import org.apache.lucene.store.Directory;
import org.apache.lucene.store.FSDirectory;
import org.apache.lucene.store.IOContext;
import org.apache.lucene.store.IndexInput;
import org.elasticsearch.common.logging.ESLoggerFactory;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;
import java.util.Random;

import static org.apache.lucene.util.LuceneTestCase.assumeTrue;
import static org.hamcrest.Matchers.is;
import static org.hamcrest.Matchers.notNullValue;
import static org.junit.Assert.assertThat;
import static org.junit.Assert.assertTrue;


public final class CorruptionUtils {
    private static Logger logger = ESLoggerFactory.getLogger("test");
    private CorruptionUtils() {}

    
    public static void corruptFile(Random random, Path... files) throws IOException {
        assertTrue("files must be non-empty", files.length > 0);
        final Path fileToCorrupt = RandomPicks.randomFrom(random, files);
        assertTrue(fileToCorrupt + " is not a file", Files.isRegularFile(fileToCorrupt));
        try (Directory dir = FSDirectory.open(fileToCorrupt.toAbsolutePath().getParent())) {
            long checksumBeforeCorruption;
            try (IndexInput input = dir.openInput(fileToCorrupt.getFileName().toString(), IOContext.DEFAULT)) {
                checksumBeforeCorruption = CodecUtil.retrieveChecksum(input);
            }
            try (FileChannel raf = FileChannel.open(fileToCorrupt, StandardOpenOption.READ, StandardOpenOption.WRITE)) {
                                raf.position(random.nextInt((int) Math.min(Integer.MAX_VALUE, raf.size())));
                long filePointer = raf.position();
                ByteBuffer bb = ByteBuffer.wrap(new byte[1]);
                raf.read(bb);
                bb.flip();

                                byte oldValue = bb.get(0);
                byte newValue = (byte) (oldValue + 1);
                bb.put(0, newValue);

                                raf.position(filePointer);
                raf.write(bb);
                logger.info("Corrupting file --  flipping at position {} from {} to {} file: {}", filePointer, Integer.toHexString(oldValue), Integer.toHexString(newValue), fileToCorrupt.getFileName());
            }
            long checksumAfterCorruption;
            long actualChecksumAfterCorruption;
            try (ChecksumIndexInput input = dir.openChecksumInput(fileToCorrupt.getFileName().toString(), IOContext.DEFAULT)) {
                assertThat(input.getFilePointer(), is(0L));
                input.seek(input.length() - 8);                 checksumAfterCorruption = input.getChecksum();
                actualChecksumAfterCorruption = input.readLong();
            }
                                    StringBuilder msg = new StringBuilder();
            msg.append("before: [").append(checksumBeforeCorruption).append("] ");
            msg.append("after: [").append(checksumAfterCorruption).append("] ");
            msg.append("checksum value after corruption: ").append(actualChecksumAfterCorruption).append("] ");
            msg.append("file: ").append(fileToCorrupt.getFileName()).append(" length: ").append(dir.fileLength(fileToCorrupt.getFileName().toString()));
            logger.info("Checksum {}", msg);
            assumeTrue("Checksum collision - " + msg.toString(),
                    checksumAfterCorruption != checksumBeforeCorruption                             || actualChecksumAfterCorruption != checksumBeforeCorruption);             assertThat("no file corrupted", fileToCorrupt, notNullValue());
        }
    }


}
