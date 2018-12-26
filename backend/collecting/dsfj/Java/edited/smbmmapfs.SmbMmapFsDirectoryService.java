

package org.elasticsearch.index.store.smbmmapfs;

import org.apache.lucene.store.Directory;
import org.apache.lucene.store.LockFactory;
import org.apache.lucene.store.MMapDirectory;
import org.elasticsearch.index.IndexSettings;
import org.elasticsearch.index.shard.ShardPath;
import org.elasticsearch.index.store.FsDirectoryService;
import org.elasticsearch.index.store.IndexStore;
import org.elasticsearch.index.store.SmbDirectoryWrapper;

import java.io.IOException;
import java.nio.file.Path;

public class SmbMmapFsDirectoryService extends FsDirectoryService {

    public SmbMmapFsDirectoryService(IndexSettings indexSettings, IndexStore indexStore, ShardPath path) {
        super(indexSettings, indexStore, path);
    }

    @Override
    protected Directory newFSDirectory(Path location, LockFactory lockFactory) throws IOException {
        logger.debug("wrapping MMapDirectory for SMB");
        return new SmbDirectoryWrapper(new MMapDirectory(location, indexSettings.getValue(INDEX_LOCK_FACTOR_SETTING)));
    }
}
