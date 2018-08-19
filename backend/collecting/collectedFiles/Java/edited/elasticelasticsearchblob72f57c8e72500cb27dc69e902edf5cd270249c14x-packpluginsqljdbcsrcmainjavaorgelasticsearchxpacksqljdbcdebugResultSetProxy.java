
package org.elasticsearch.xpack.sql.jdbc.debug;

import java.sql.ResultSetMetaData;
import java.sql.Statement;

class ResultSetProxy extends DebuggingInvoker {

    ResultSetProxy(DebugLog log, Object target, Object parent) {
        super(log, target, parent);
    }

    @Override
    protected Object postProcess(Object result, Object proxy) {
        if (result instanceof ResultSetMetaData) {
            return Debug.proxy(new ResultSetMetaDataProxy(log, result));
        }
        if (result instanceof Statement) {
            return parent;
        }
        return result;
    }
}
