
package org.elasticsearch.xpack.sql.capabilities;

import org.elasticsearch.xpack.sql.ServerSqlException;

import java.util.Locale;

import static java.lang.String.format;


public class UnresolvedException extends ServerSqlException {
    public UnresolvedException(String action, Object target) {
        super(format(Locale.ROOT, "Invalid call to %s on an unresolved object %s", action, target));
    }
}
