

package org.elasticsearch.index.translog;

import joptsimple.OptionParser;
import joptsimple.OptionSet;
import joptsimple.OptionSpec;
import org.apache.lucene.index.DirectoryReader;
import org.apache.lucene.index.IndexCommit;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.index.IndexWriterConfig;
import org.apache.lucene.index.NoMergePolicy;
import org.apache.lucene.store.Directory;
import org.apache.lucene.store.FSDirectory;
import org.apache.lucene.store.Lock;
import org.apache.lucene.store.LockObtainFailedException;
import org.apache.lucene.store.NativeFSLockFactory;
import org.apache.lucene.store.OutputStreamDataOutput;
import org.apache.lucene.util.BytesRef;
import org.elasticsearch.core.internal.io.IOUtils;
import org.elasticsearch.ElasticsearchException;
import org.elasticsearch.cli.EnvironmentAwareCommand;
import org.elasticsearch.cli.Terminal;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.UUIDs;
import org.elasticsearch.common.io.PathUtils;
import org.elasticsearch.env.Environment;
import org.elasticsearch.index.IndexNotFoundException;
import org.elasticsearch.index.engine.Engine;
import org.elasticsearch.index.seqno.SequenceNumbers;

import java.io.IOException;
import java.nio.channels.Channels;
import java.nio.channels.FileChannel;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.nio.file.StandardOpenOption;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

public class TruncateTranslogCommand extends EnvironmentAwareCommand {

    private final OptionSpec<String> translogFolder;
    private final OptionSpec<Void> batchMode;

    public TruncateTranslogCommand() {
        super("Truncates a translog to create a new, empty translog");
        this.translogFolder = parser.acceptsAll(Arrays.asList("d", "dir"),
                "Translog Directory location on disk")
                .withRequiredArg()
                .required();
        this.batchMode = parser.acceptsAll(Arrays.asList("b", "batch"),
                "Enable batch mode explicitly, automatic confirmation of warnings");
    }

        public OptionParser getParser() {
        return this.parser;
    }

    @Override
    protected void printAdditionalHelp(Terminal terminal) {
        terminal.println("This tool truncates the translog and translog");
        terminal.println("checkpoint files to create a new translog");
    }

    @SuppressForbidden(reason = "Necessary to use the path passed in")
    private Path getTranslogPath(OptionSet options) {
        return PathUtils.get(translogFolder.value(options), "", "");
    }

    @Override
    protected void execute(Terminal terminal, OptionSet options, Environment env) throws Exception {
        boolean batch = options.has(batchMode);

        Path translogPath = getTranslogPath(options);
        Path idxLocation = translogPath.getParent().resolve("index");

        if (Files.exists(translogPath) == false || Files.isDirectory(translogPath) == false) {
            throw new ElasticsearchException("translog directory [" + translogPath + "], must exist and be a directory");
        }

        if (Files.exists(idxLocation) == false || Files.isDirectory(idxLocation) == false) {
            throw new ElasticsearchException("unable to find a shard at [" + idxLocation + "], which must exist and be a directory");
        }
        try (Directory dir = FSDirectory.open(idxLocation, NativeFSLockFactory.INSTANCE)) {
            final String historyUUID = UUIDs.randomBase64UUID();
            final Map<String, String> commitData;
                        try (Lock writeLock = dir.obtainLock(IndexWriter.WRITE_LOCK_NAME)) {
                Set<Path> translogFiles;
                try {
                    terminal.println("Checking existing translog files");
                    translogFiles = filesInDirectory(translogPath);
                } catch (IOException e) {
                    terminal.println("encountered IOException while listing directory, aborting...");
                    throw new ElasticsearchException("failed to find existing translog files", e);
                }

                                warnAboutDeletingFiles(terminal, translogFiles, batch);

                List<IndexCommit> commits;
                try {
                    terminal.println("Reading translog UUID information from Lucene commit from shard at [" + idxLocation + "]");
                    commits = DirectoryReader.listCommits(dir);
                } catch (IndexNotFoundException infe) {
                    throw new ElasticsearchException("unable to find a valid shard at [" + idxLocation + "]", infe);
                }

                                commitData = new HashMap<>(commits.get(commits.size() - 1).getUserData());
                String translogGeneration = commitData.get(Translog.TRANSLOG_GENERATION_KEY);
                String translogUUID = commitData.get(Translog.TRANSLOG_UUID_KEY);
                final long globalCheckpoint;
                                                if (commitData.containsKey(SequenceNumbers.MAX_SEQ_NO)) {
                    globalCheckpoint = Long.parseLong(commitData.get(SequenceNumbers.MAX_SEQ_NO));
                                        commitData.put(SequenceNumbers.LOCAL_CHECKPOINT_KEY, Long.toString(globalCheckpoint));
                } else {
                    globalCheckpoint = SequenceNumbers.UNASSIGNED_SEQ_NO;
                }
                if (translogGeneration == null || translogUUID == null) {
                    throw new ElasticsearchException("shard must have a valid translog generation and UUID but got: [{}] and: [{}]",
                        translogGeneration, translogUUID);
                }
                terminal.println("Translog Generation: " + translogGeneration);
                terminal.println("Translog UUID      : " + translogUUID);
                terminal.println("History UUID      : " + historyUUID);

                Path tempEmptyCheckpoint = translogPath.resolve("temp-" + Translog.CHECKPOINT_FILE_NAME);
                Path realEmptyCheckpoint = translogPath.resolve(Translog.CHECKPOINT_FILE_NAME);
                Path tempEmptyTranslog = translogPath.resolve("temp-" + Translog.TRANSLOG_FILE_PREFIX +
                    translogGeneration + Translog.TRANSLOG_FILE_SUFFIX);
                Path realEmptyTranslog = translogPath.resolve(Translog.TRANSLOG_FILE_PREFIX +
                    translogGeneration + Translog.TRANSLOG_FILE_SUFFIX);

                                long gen = Long.parseLong(translogGeneration);
                int translogLen = writeEmptyTranslog(tempEmptyTranslog, translogUUID);
                writeEmptyCheckpoint(tempEmptyCheckpoint, translogLen, gen, globalCheckpoint);

                terminal.println("Removing existing translog files");
                IOUtils.rm(translogFiles.toArray(new Path[]{}));

                terminal.println("Creating new empty checkpoint at [" + realEmptyCheckpoint + "]");
                Files.move(tempEmptyCheckpoint, realEmptyCheckpoint, StandardCopyOption.ATOMIC_MOVE);
                terminal.println("Creating new empty translog at [" + realEmptyTranslog + "]");
                Files.move(tempEmptyTranslog, realEmptyTranslog, StandardCopyOption.ATOMIC_MOVE);

                                IOUtils.fsync(translogPath, true);
            }

            terminal.println("Marking index with the new history uuid");
                        IndexWriterConfig iwc = new IndexWriterConfig(null)
                .setCommitOnClose(false)
                                                                .setMergePolicy(NoMergePolicy.INSTANCE)
                .setOpenMode(IndexWriterConfig.OpenMode.APPEND);
            try (IndexWriter writer = new IndexWriter(dir, iwc)) {
                Map<String, String> newCommitData = new HashMap<>(commitData);
                newCommitData.put(Engine.HISTORY_UUID_KEY, historyUUID);
                writer.setLiveCommitData(newCommitData.entrySet());
                writer.commit();
            }
        } catch (LockObtainFailedException lofe) {
            throw new ElasticsearchException("Failed to lock shard's directory at [" + idxLocation + "], is Elasticsearch still running?");
        }

        terminal.println("Done.");
    }

    
    static void writeEmptyCheckpoint(Path filename, int translogLength, long translogGeneration, long globalCheckpoint) throws IOException {
        Checkpoint emptyCheckpoint = Checkpoint.emptyTranslogCheckpoint(translogLength, translogGeneration,
            globalCheckpoint, translogGeneration);
        Checkpoint.write(FileChannel::open, filename, emptyCheckpoint,
            StandardOpenOption.WRITE, StandardOpenOption.READ, StandardOpenOption.CREATE_NEW);
                IOUtils.fsync(filename, false);
    }

    
    public static int writeEmptyTranslog(Path filename, String translogUUID) throws IOException {
        final BytesRef translogRef = new BytesRef(translogUUID);
        try (FileChannel fc = FileChannel.open(filename, StandardOpenOption.WRITE, StandardOpenOption.READ, StandardOpenOption.CREATE_NEW);
                OutputStreamDataOutput out = new OutputStreamDataOutput(Channels.newOutputStream(fc))) {
            TranslogWriter.writeHeader(out, translogRef);
            fc.force(true);
        }
        return TranslogWriter.getHeaderLength(translogRef.length);
    }

    
    public static void warnAboutDeletingFiles(Terminal terminal, Set<Path> files, boolean batchMode) {
        terminal.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        terminal.println("!   WARNING: Elasticsearch MUST be stopped before running this tool   !");
        terminal.println("!                                                                     !");
        terminal.println("!   WARNING:    Documents inside of translog files will be lost       !");
        terminal.println("!                                                                     !");
        terminal.println("!   WARNING:          The following files will be DELETED!            !");
        terminal.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
        for (Path file : files) {
            terminal.println("--> " + file);
        }
        terminal.println("");
        if (batchMode == false) {
            String text = terminal.readText("Continue and DELETE files? [y/N] ");
            if (!text.equalsIgnoreCase("y")) {
                throw new ElasticsearchException("aborted by user");
            }
        }
    }

    
    public static Set<Path> filesInDirectory(Path directory) throws IOException {
        Set<Path> files = new HashSet<>();
        try (DirectoryStream<Path> stream = Files.newDirectoryStream(directory)) {
            for (Path file : stream) {
                files.add(file);
            }
        }
        return files;
    }

}
