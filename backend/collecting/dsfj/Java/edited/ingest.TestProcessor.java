

package org.elasticsearch.ingest;

import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Consumer;


public class TestProcessor implements Processor {

    private final String type;
    private final String tag;
    private final Consumer<IngestDocument> ingestDocumentConsumer;
    private final AtomicInteger invokedCounter = new AtomicInteger();

    public TestProcessor(Consumer<IngestDocument> ingestDocumentConsumer) {
        this(null, "test-processor", ingestDocumentConsumer);
    }

    public TestProcessor(String tag, String type, Consumer<IngestDocument> ingestDocumentConsumer) {
        this.ingestDocumentConsumer = ingestDocumentConsumer;
        this.type = type;
        this.tag = tag;
    }

    @Override
    public void execute(IngestDocument ingestDocument) throws Exception {
        invokedCounter.incrementAndGet();
        ingestDocumentConsumer.accept(ingestDocument);
    }

    @Override
    public String getType() {
        return type;
    }

    @Override
    public String getTag() {
        return tag;
    }

    public int getInvokedCounter() {
        return invokedCounter.get();
    }

    public static final class Factory implements Processor.Factory {
        @Override
        public TestProcessor create(Map<String, Processor.Factory> registry, String processorTag,
                                    Map<String, Object> config) throws Exception {
            return new TestProcessor(processorTag, "test-processor", ingestDocument -> {});
        }
    }
}
