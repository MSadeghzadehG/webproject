

package org.elasticsearch.bootstrap;

import org.apache.lucene.util.Constants;
import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.env.Environment;
import org.elasticsearch.plugins.Platforms;
import org.elasticsearch.plugins.PluginInfo;
import org.elasticsearch.plugins.PluginsService;

import java.io.Closeable;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.atomic.AtomicBoolean;


final class Spawner implements Closeable {

    
    private final List<Process> processes = new ArrayList<>();
    private AtomicBoolean spawned = new AtomicBoolean();

    @Override
    public void close() throws IOException {
        IOUtils.close(() -> processes.stream().map(s -> (Closeable) s::destroy).iterator());
    }

    
    void spawnNativePluginControllers(final Environment environment) throws IOException {
        if (!spawned.compareAndSet(false, true)) {
            throw new IllegalStateException("native controllers already spawned");
        }
        spawnControllers(environment.pluginsFile(), "plugins", environment.tmpFile());
        spawnControllers(environment.modulesFile(), "modules", environment.tmpFile());
    }

    
    private void spawnControllers(Path pluginsDir, String type, Path tmpDir) throws IOException {
        if (!Files.exists(pluginsDir)) {
            throw new IllegalStateException(type + " directory [" + pluginsDir + "] not found");
        }
        
        List<Path> paths = PluginsService.findPluginDirs(pluginsDir);
        for (Path plugin : paths) {
            final PluginInfo info = PluginInfo.readFromProperties(plugin);
            final Path spawnPath = Platforms.nativeControllerPath(plugin);
            if (!Files.isRegularFile(spawnPath)) {
                continue;
            }
            if (!info.hasNativeController()) {
                final String message = String.format(
                    Locale.ROOT,
                    "plugin [%s] does not have permission to fork native controller",
                    plugin.getFileName());
                throw new IllegalArgumentException(message);
            }
            final Process process = spawnNativePluginController(spawnPath, tmpDir);
            processes.add(process);
        }
    }

    
    private Process spawnNativePluginController(
            final Path spawnPath,
            final Path tmpPath) throws IOException {
        final String command;
        if (Constants.WINDOWS) {
            
            command = Natives.getShortPathName(spawnPath.toString());
        } else {
            command = spawnPath.toString();
        }
        final ProcessBuilder pb = new ProcessBuilder(command);

                pb.environment().clear();
        pb.environment().put("TMPDIR", tmpPath.toString());

                return pb.start();
    }

    
    List<Process> getProcesses() {
        return Collections.unmodifiableList(processes);
    }

}
