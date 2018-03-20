
package org.elasticsearch.client.benchmark.metrics;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;


public final class SampleRecorder {
    private final List<Sample> samples;

    public SampleRecorder(int iterations) {
        this.samples = new ArrayList<>(iterations);
    }

    public void addSample(Sample sample) {
        samples.add(sample);
    }

    public List<Sample> getSamples() {
        return Collections.unmodifiableList(samples);
    }
}
