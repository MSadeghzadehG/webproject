

package org.elasticsearch.cluster.block;


import java.util.EnumSet;

public enum ClusterBlockLevel {
    READ,
    WRITE,
    METADATA_READ,
    METADATA_WRITE;

    public static final EnumSet<ClusterBlockLevel> ALL = EnumSet.allOf(ClusterBlockLevel.class);
    public static final EnumSet<ClusterBlockLevel> READ_WRITE = EnumSet.of(READ, WRITE);
}
