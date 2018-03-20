

package org.elasticsearch.action.admin.cluster.node.info;

import org.elasticsearch.common.io.stream.StreamInput;
import org.elasticsearch.common.io.stream.StreamOutput;
import org.elasticsearch.common.io.stream.Writeable;
import org.elasticsearch.common.xcontent.ToXContent.Params;
import org.elasticsearch.common.xcontent.ToXContentFragment;
import org.elasticsearch.common.xcontent.XContentBuilder;
import org.elasticsearch.plugins.PluginInfo;

import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;


public class PluginsAndModules implements Writeable, ToXContentFragment {
    private final List<PluginInfo> plugins;
    private final List<PluginInfo> modules;

    public PluginsAndModules(List<PluginInfo> plugins, List<PluginInfo> modules) {
        this.plugins = Collections.unmodifiableList(plugins);
        this.modules = Collections.unmodifiableList(modules);
    }

    public PluginsAndModules(StreamInput in) throws IOException {
        this.plugins = Collections.unmodifiableList(in.readList(PluginInfo::new));
        this.modules = Collections.unmodifiableList(in.readList(PluginInfo::new));
    }

    @Override
    public void writeTo(StreamOutput out) throws IOException {
        out.writeList(plugins);
        out.writeList(modules);
    }

    
    public List<PluginInfo> getPluginInfos() {
        List<PluginInfo> plugins = new ArrayList<>(this.plugins);
        Collections.sort(plugins, Comparator.comparing(PluginInfo::getName));
        return plugins;
    }

    
    public List<PluginInfo> getModuleInfos() {
        List<PluginInfo> modules = new ArrayList<>(this.modules);
        Collections.sort(modules, Comparator.comparing(PluginInfo::getName));
        return modules;
    }

    public void addPlugin(PluginInfo info) {
        plugins.add(info);
    }

    public void addModule(PluginInfo info) {
        modules.add(info);
    }

    @Override
    public XContentBuilder toXContent(XContentBuilder builder, Params params) throws IOException {
        builder.startArray("plugins");
        for (PluginInfo pluginInfo : getPluginInfos()) {
            pluginInfo.toXContent(builder, params);
        }
        builder.endArray();
                builder.startArray("modules");
        for (PluginInfo moduleInfo : getModuleInfos()) {
            moduleInfo.toXContent(builder, params);
        }
        builder.endArray();

        return builder;
    }
}
