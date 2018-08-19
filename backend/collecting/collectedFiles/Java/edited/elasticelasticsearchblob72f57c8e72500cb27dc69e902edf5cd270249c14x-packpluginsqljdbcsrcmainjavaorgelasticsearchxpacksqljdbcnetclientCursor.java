
package org.elasticsearch.xpack.sql.jdbc.net.client;

import java.sql.SQLException;
import java.util.List;

import org.elasticsearch.xpack.sql.jdbc.net.protocol.ColumnInfo;

public interface Cursor {

    List<ColumnInfo> columns();

    default int columnSize() {
        return columns().size();
    }

    boolean next() throws SQLException;

    Object column(int column);

    
    int batchSize();

    void close() throws SQLException;
}
