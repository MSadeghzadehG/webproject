
package org.elasticsearch.index.snapshots.blobstore;

import org.elasticsearch.index.snapshots.blobstore.BlobStoreIndexShardSnapshot.FileInfo;

import java.util.HashMap;
import java.util.List;
import java.util.Map;


public class SnapshotFiles {

    private final String snapshot;

    private final List<FileInfo> indexFiles;

    private Map<String, FileInfo> physicalFiles = null;

    public String snapshot() {
        return snapshot;
    }

    public SnapshotFiles(String snapshot, List<FileInfo> indexFiles ) {
        this.snapshot = snapshot;
        this.indexFiles = indexFiles;
    }

    
    public List<FileInfo> indexFiles() {
        return indexFiles;
    }

    
    public boolean containPhysicalIndexFile(String physicalName) {
        return findPhysicalIndexFile(physicalName) != null;
    }

    
    public FileInfo findPhysicalIndexFile(String physicalName) {
        if (physicalFiles == null) {
            Map<String, FileInfo> files = new HashMap<>();
            for(FileInfo fileInfo : indexFiles) {
                files.put(fileInfo.physicalName(), fileInfo);
            }
            this.physicalFiles = files;
        }
        return physicalFiles.get(physicalName);
    }

}
