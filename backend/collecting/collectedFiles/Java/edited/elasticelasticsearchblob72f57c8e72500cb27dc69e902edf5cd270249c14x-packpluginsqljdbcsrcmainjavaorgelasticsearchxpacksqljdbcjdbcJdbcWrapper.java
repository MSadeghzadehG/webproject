
package org.elasticsearch.xpack.sql.jdbc.jdbc;

import java.sql.SQLException;
import java.sql.Wrapper;

interface JdbcWrapper extends Wrapper {

    @Override
    default boolean isWrapperFor(Class<?> iface) throws SQLException {
        return iface != null && iface.isAssignableFrom(getClass());
    }

    @SuppressWarnings("unchecked")
    @Override
    default <T> T unwrap(Class<T> iface) throws SQLException {
        if (isWrapperFor(iface)) {
            return (T) this;
        }
        throw new SQLException();
    }
}
