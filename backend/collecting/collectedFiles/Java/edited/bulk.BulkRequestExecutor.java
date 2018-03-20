
package org.elasticsearch.client.benchmark.ops.bulk;

import java.util.List;

public interface BulkRequestExecutor {
    boolean bulkIndex(List<String> bulkData);
}
