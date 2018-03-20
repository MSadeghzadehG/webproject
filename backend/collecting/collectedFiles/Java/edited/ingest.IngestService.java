

package org.elasticsearch.ingest;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.env.Environment;
import org.elasticsearch.index.analysis.AnalysisRegistry;
import org.elasticsearch.plugins.IngestPlugin;
import org.elasticsearch.script.ScriptService;
import org.elasticsearch.threadpool.ThreadPool;


public class IngestService {
    private final PipelineStore pipelineStore;
    private final PipelineExecutionService pipelineExecutionService;

    public IngestService(Settings settings, ThreadPool threadPool,
                         Environment env, ScriptService scriptService, AnalysisRegistry analysisRegistry,
                         List<IngestPlugin> ingestPlugins) {
        Processor.Parameters parameters = new Processor.Parameters(env, scriptService,
            analysisRegistry, threadPool.getThreadContext());
        Map<String, Processor.Factory> processorFactories = new HashMap<>();
        for (IngestPlugin ingestPlugin : ingestPlugins) {
            Map<String, Processor.Factory> newProcessors = ingestPlugin.getProcessors(parameters);
            for (Map.Entry<String, Processor.Factory> entry : newProcessors.entrySet()) {
                if (processorFactories.put(entry.getKey(), entry.getValue()) != null) {
                    throw new IllegalArgumentException("Ingest processor [" + entry.getKey() + "] is already registered");
                }
            }
        }
        this.pipelineStore = new PipelineStore(settings, Collections.unmodifiableMap(processorFactories));
        this.pipelineExecutionService = new PipelineExecutionService(pipelineStore, threadPool);
    }

    public PipelineStore getPipelineStore() {
        return pipelineStore;
    }

    public PipelineExecutionService getPipelineExecutionService() {
        return pipelineExecutionService;
    }

    public IngestInfo info() {
        Map<String, Processor.Factory> processorFactories = pipelineStore.getProcessorFactories();
        List<ProcessorInfo> processorInfoList = new ArrayList<>(processorFactories.size());
        for (Map.Entry<String, Processor.Factory> entry : processorFactories.entrySet()) {
            processorInfoList.add(new ProcessorInfo(entry.getKey()));
        }
        return new IngestInfo(processorInfoList);
    }
}
