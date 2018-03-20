

package org.elasticsearch.index.store;

import org.apache.lucene.store.Directory;
import org.apache.lucene.store.FSDirectory;
import org.apache.lucene.store.FileSwitchDirectory;
import org.apache.lucene.store.LockFactory;
import org.apache.lucene.store.MMapDirectory;
import org.apache.lucene.store.NIOFSDirectory;
import org.apache.lucene.store.NativeFSLockFactory;
import org.apache.lucene.store.SimpleFSDirectory;
import org.apache.lucene.store.SimpleFSLockFactory;
import org.elasticsearch.common.inject.Inject;
import org.elasticsearch.common.settings.Setting;
import org.elasticsearch.common.settings.Setting.Property;
import org.elasticsearch.index.IndexModule;
import org.elasticsearch.index.IndexSettings;
import org.elasticsearch.index.shard.ShardPath;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashSet;
import java.util.Set;

public class FsDirectoryService extends DirectoryService {

    protected final IndexStore indexStore;
    public static final Setting<LockFactory> INDEX_LOCK_FACTOR_SETTING = new Setting<>("index.store.fs.fs_lock", "native", (s) -> {
        switch (s) {
            case "native":
                return NativeFSLockFactory.INSTANCE;
            case "simple":
                return SimpleFSLockFactory.INSTANCE;
            default:
                throw new IllegalArgumentException("unrecognized [index.store.fs.fs_lock] \"" + s + "\": must be native or simple");
        }     }, Property.IndexScope, Property.NodeScope);

    private final ShardPath path;

    @Inject
    public FsDirectoryService(IndexSettings indexSettings, IndexStore indexStore, ShardPath path) {
        super(path.getShardId(), indexSettings);
        this.path = path;
        this.indexStore = indexStore;
    }

    @Override
    public Directory newDirectory() throws IOException {
        final Path location = path.resolveIndex();
        final LockFactory lockFactory = indexSettings.getValue(INDEX_LOCK_FACTOR_SETTING);
        Files.createDirectories(location);
        Directory wrapped = newFSDirectory(location, lockFactory);
        Set<String> preLoadExtensions = new HashSet<>(
                indexSettings.getValue(IndexModule.INDEX_STORE_PRE_LOAD_SETTING));
        wrapped = setPreload(wrapped, location, lockFactory, preLoadExtensions);
        return wrapped;
    }

    protected Directory newFSDirectory(Path location, LockFactory lockFactory) throws IOException {
        final String storeType = indexSettings.getSettings().get(IndexModule.INDEX_STORE_TYPE_SETTING.getKey(),
            IndexModule.Type.FS.getSettingsKey());
        if (IndexModule.Type.FS.match(storeType)) {
            return FSDirectory.open(location, lockFactory);         } else if (IndexModule.Type.SIMPLEFS.match(storeType)) {
            return new SimpleFSDirectory(location, lockFactory);
        } else if (IndexModule.Type.NIOFS.match(storeType)) {
            return new NIOFSDirectory(location, lockFactory);
        } else if (IndexModule.Type.MMAPFS.match(storeType)) {
            return new MMapDirectory(location, lockFactory);
        }
        throw new IllegalArgumentException("No directory found for type [" + storeType + "]");
    }

    private static Directory setPreload(Directory directory, Path location, LockFactory lockFactory,
            Set<String> preLoadExtensions) throws IOException {
        if (preLoadExtensions.isEmpty() == false
                && directory instanceof MMapDirectory
                && ((MMapDirectory) directory).getPreload() == false) {
            if (preLoadExtensions.contains("*")) {
                ((MMapDirectory) directory).setPreload(true);
                return directory;
            }
            MMapDirectory primary = new MMapDirectory(location, lockFactory);
            primary.setPreload(true);
            return new FileSwitchDirectory(preLoadExtensions, primary, directory, true) {
                @Override
                public String[] listAll() throws IOException {
                                        return primary.listAll();
                }
            };
        }
        return directory;
    }
}
