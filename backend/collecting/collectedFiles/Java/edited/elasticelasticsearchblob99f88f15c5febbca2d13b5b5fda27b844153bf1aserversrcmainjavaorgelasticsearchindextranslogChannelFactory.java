
package org.elasticsearch.index.translog;

import java.io.IOException;
import java.nio.channels.FileChannel;
import java.nio.file.OpenOption;
import java.nio.file.Path;
import java.nio.file.StandardOpenOption;


@FunctionalInterface
public interface ChannelFactory {
    default FileChannel open(Path path) throws IOException {
        return open(path, StandardOpenOption.WRITE, StandardOpenOption.READ, StandardOpenOption.CREATE_NEW);
    }

    FileChannel open(Path path, OpenOption... options) throws IOException;
}
