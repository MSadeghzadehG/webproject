
package org.elasticsearch.xpack.security.support;

import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.env.Environment;

import java.io.IOException;
import java.io.UncheckedIOException;
import java.io.Writer;
import java.nio.charset.StandardCharsets;
import java.nio.file.AtomicMoveNotSupportedException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.PosixFileAttributeView;
import java.nio.file.attribute.PosixFileAttributes;
import java.util.Locale;
import java.util.Map;
import java.util.function.Function;

import static java.nio.file.StandardCopyOption.ATOMIC_MOVE;
import static java.nio.file.StandardCopyOption.REPLACE_EXISTING;
import static java.nio.file.StandardOpenOption.CREATE;
import static java.nio.file.StandardOpenOption.TRUNCATE_EXISTING;
import static java.nio.file.StandardOpenOption.WRITE;

public class SecurityFiles {

    private SecurityFiles() {
    }

    
    public static <K, V> void writeFileAtomically(final Path path, final Map<K, V> map, final Function<Map.Entry<K, V>, String> transform) {
        Path tempFile = null;
        try {
            tempFile = Files.createTempFile(path.getParent(), path.getFileName().toString(), "tmp");
            try (Writer writer = Files.newBufferedWriter(tempFile, StandardCharsets.UTF_8, CREATE, TRUNCATE_EXISTING, WRITE)) {
                for (final Map.Entry<K, V> entry : map.entrySet()) {
                    final StringBuilder sb = new StringBuilder();
                    final String line = sb.append(transform.apply(entry)).append(System.lineSeparator()).toString();
                    writer.write(line);
                }
            }
                        if (Files.exists(path)) {
                boolean supportsPosixAttributes =
                        Environment.getFileStore(path).supportsFileAttributeView(PosixFileAttributeView.class);
                if (supportsPosixAttributes) {
                    setPosixAttributesOnTempFile(path, tempFile);
                }
            }

            try {
                Files.move(tempFile, path, REPLACE_EXISTING, ATOMIC_MOVE);
            } catch (final AtomicMoveNotSupportedException e) {
                Files.move(tempFile, path, REPLACE_EXISTING);
            }
        } catch (final IOException e) {
            throw new UncheckedIOException(String.format(Locale.ROOT, "could not write file [%s]", path.toAbsolutePath()), e);
        } finally {
                        IOUtils.deleteFilesIgnoringExceptions(tempFile);
        }
    }

    static void setPosixAttributesOnTempFile(Path path, Path tempFile) throws IOException {
        PosixFileAttributes attributes = Files.getFileAttributeView(path, PosixFileAttributeView.class).readAttributes();
        PosixFileAttributeView tempFileView = Files.getFileAttributeView(tempFile, PosixFileAttributeView.class);

        tempFileView.setPermissions(attributes.permissions());

                        try {
            tempFileView.setOwner(attributes.owner());
        } catch (Exception e) {
        }

        try {
            tempFileView.setGroup(attributes.group());
        } catch (Exception e) {
        }
    }
}
