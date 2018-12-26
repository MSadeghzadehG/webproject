

package org.elasticsearch.index.engine;

import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.index.MergePolicy;
import org.apache.lucene.index.SegmentCommitInfo;
import org.elasticsearch.index.mapper.ParsedDocument;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.atomic.AtomicReference;
import java.util.stream.Collectors;
import java.util.stream.StreamSupport;

import static org.hamcrest.Matchers.containsString;
import static org.hamcrest.Matchers.hasToString;
import static org.hamcrest.Matchers.instanceOf;

public class EvilInternalEngineTests extends EngineTestCase {

    public void testOutOfMemoryErrorWhileMergingIsRethrownAndIsUncaught() throws IOException, InterruptedException {
        engine.close();
        final AtomicReference<Throwable> maybeFatal = new AtomicReference<>();
        final CountDownLatch latch = new CountDownLatch(1);
        final Thread.UncaughtExceptionHandler uncaughtExceptionHandler = Thread.getDefaultUncaughtExceptionHandler();
        try {
            
            Thread.setDefaultUncaughtExceptionHandler((t, e) -> {
                maybeFatal.set(e);
                latch.countDown();
            });
            final AtomicReference<List<SegmentCommitInfo>> segmentsReference = new AtomicReference<>();

            try (Engine e = createEngine(
                    defaultSettings,
                    store,
                    primaryTranslogDir,
                    newMergePolicy(),
                    (directory, iwc) -> new IndexWriter(directory, iwc) {
                        @Override
                        public void merge(final MergePolicy.OneMerge merge) throws IOException {
                            throw new OutOfMemoryError("640K ought to be enough for anybody");
                        }

                        @Override
                        public synchronized MergePolicy.OneMerge getNextMerge() {
                            
                            if (segmentsReference.get() == null) {
                                return super.getNextMerge();
                            } else {
                                final List<SegmentCommitInfo> segments = segmentsReference.getAndSet(null);
                                return new MergePolicy.OneMerge(segments);
                            }
                        }
                    },
                    null,
                    null)) {
                                final ParsedDocument doc1 = testParsedDocument("1", null, testDocumentWithTextField(), B_1, null);
                e.index(indexForDoc(doc1));
                e.flush();
                final List<SegmentCommitInfo> segments =
                        StreamSupport.stream(e.getLastCommittedSegmentInfos().spliterator(), false).collect(Collectors.toList());
                segmentsReference.set(segments);
                                e.forceMerge(randomBoolean(), 0, false, false, false);
                
                latch.await();
                assertNotNull(maybeFatal.get());
                assertThat(maybeFatal.get(), instanceOf(OutOfMemoryError.class));
                assertThat(maybeFatal.get(), hasToString(containsString("640K ought to be enough for anybody")));
            }
        } finally {
            Thread.setDefaultUncaughtExceptionHandler(uncaughtExceptionHandler);
        }
    }


}
