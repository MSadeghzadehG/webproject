

package org.elasticsearch.env;

import org.apache.lucene.util.Constants;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.io.PathUtils;

import java.io.IOException;
import java.nio.file.FileStore;
import java.nio.file.FileSystemException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.attribute.FileAttributeView;
import java.nio.file.attribute.FileStoreAttributeView;
import java.util.Arrays;
import java.util.List;


class ESFileStore extends FileStore {
    
    final FileStore in;
    private int majorDeviceNumber;
    private int minorDeviceNumber;
    
    @SuppressForbidden(reason = "tries to determine if disk is spinning")
            ESFileStore(final FileStore in) {
        this.in = in;
        if (Constants.LINUX) {
            try {
                final List<String> lines = Files.readAllLines(PathUtils.get("/proc/self/mountinfo"));
                for (final String line : lines) {
                    final String[] fields = line.trim().split("\\s+");
                    final String mountPoint = fields[4];
                    if (mountPoint.equals(getMountPointLinux(in))) {
                        final String[] deviceNumbers = fields[2].split(":");
                        majorDeviceNumber = Integer.parseInt(deviceNumbers[0]);
                        minorDeviceNumber = Integer.parseInt(deviceNumbers[1]);
                        break;
                    }
                }
            } catch (final Exception e) {
                majorDeviceNumber = -1;
                minorDeviceNumber = -1;
            }
        } else {
            majorDeviceNumber = -1;
            minorDeviceNumber = -1;
        }
    }

        private static String getMountPointLinux(final FileStore store) {
        String desc = store.toString();
        int index = desc.lastIndexOf(" (");
        if (index != -1) {
            return desc.substring(0, index);
        } else {
            return desc;
        }
    }

    @Override
    public String name() {
        return in.name();
    }

    @Override
    public String type() {
        return in.type();
    }

    @Override
    public boolean isReadOnly() {
        return in.isReadOnly();
    }

    @Override
    public long getTotalSpace() throws IOException {
        long result = in.getTotalSpace();
        if (result < 0) {
                        result = Long.MAX_VALUE;
        }
        return result;
    }

    @Override
    public long getUsableSpace() throws IOException {
        long result = in.getUsableSpace();
        if (result < 0) {
                        result = Long.MAX_VALUE;
        }
        return result;
    }

    @Override
    public long getUnallocatedSpace() throws IOException {
        long result = in.getUnallocatedSpace();
        if (result < 0) {
                        result = Long.MAX_VALUE;
        }
        return result;
    }

    @Override
    public boolean supportsFileAttributeView(Class<? extends FileAttributeView> type) {
        return in.supportsFileAttributeView(type);
    }

    @Override
    public boolean supportsFileAttributeView(String name) {
        if ("lucene".equals(name)) {
            return true;
        } else {
            return in.supportsFileAttributeView(name);
        }
    }

    @Override
    public <V extends FileStoreAttributeView> V getFileStoreAttributeView(Class<V> type) {
        return in.getFileStoreAttributeView(type);
    }

    @Override
    public Object getAttribute(String attribute) throws IOException {
        switch(attribute) {
                        case "lucene:major_device_number": return majorDeviceNumber;
            case "lucene:minor_device_number": return minorDeviceNumber;
            default: return in.getAttribute(attribute);
        }
    }

    @Override
    public String toString() {
        return in.toString();
    }
}
