
package org.elasticsearch.xpack.sql.jdbc.debug;

import java.sql.Connection;
import java.sql.ParameterMetaData;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;

final class StatementProxy extends DebuggingInvoker {

    StatementProxy(DebugLog log, Object target, Object con) {
        super(log, target, con);
    }

    @Override
    protected Object postProcess(Object result, Object proxy) {
        if (result instanceof Connection) {
            return parent;
        }
        if (result instanceof ResultSet) {
            return Debug.proxy(new ResultSetProxy(log, result, proxy));
        }
        if (result instanceof ParameterMetaData) {
            return Debug.proxy(new ParameterMetaDataProxy(log, result));
        }
        if (result instanceof ResultSetMetaData) {
            return Debug.proxy(new ResultSetMetaDataProxy(log, result));
        }

        return result;
    }
}
