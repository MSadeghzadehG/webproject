
package org.elasticsearch.test.rest;

import org.elasticsearch.common.Nullable;
import org.elasticsearch.common.io.stream.BytesStreamOutput;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.common.xcontent.XContentType;
import org.elasticsearch.rest.AbstractRestChannel;
import org.elasticsearch.rest.RestRequest;
import org.elasticsearch.rest.RestResponse;
import org.elasticsearch.rest.RestStatus;

import java.io.IOException;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

public final class FakeRestChannel extends AbstractRestChannel {
    private final CountDownLatch latch;
    private final AtomicInteger responses = new AtomicInteger();
    private final AtomicInteger errors = new AtomicInteger();
    private RestResponse capturedRestResponse;

    public FakeRestChannel(RestRequest request, boolean detailedErrorsEnabled, int responseCount) {
        super(request, detailedErrorsEnabled);
        this.latch = new CountDownLatch(responseCount);
    }

    @Override
    public XContentBuilder newBuilder() throws IOException {
        return super.newBuilder();
    }

    @Override
    public XContentBuilder newErrorBuilder() throws IOException {
        return super.newErrorBuilder();
    }

    @Override
    public XContentBuilder newBuilder(@Nullable XContentType requestContentType, boolean useFiltering) throws IOException {
        return super.newBuilder(requestContentType, useFiltering);
    }

    @Override
    protected BytesStreamOutput newBytesOutput() {
        return super.newBytesOutput();
    }

    @Override
    public RestRequest request() {
        return super.request();
    }

    @Override
    public void sendResponse(RestResponse response) {
        this.capturedRestResponse = response;
        if (response.status() == RestStatus.OK) {
            responses.incrementAndGet();
        } else {
            errors.incrementAndGet();
        }
        latch.countDown();
    }
    
    public RestResponse capturedResponse() {
        return capturedRestResponse;
    }

    public boolean await() throws InterruptedException {
        return latch.await(10, TimeUnit.SECONDS);
    }

    public AtomicInteger responses() {
        return responses;
    }

    public AtomicInteger errors() {
        return errors;
    }
}
