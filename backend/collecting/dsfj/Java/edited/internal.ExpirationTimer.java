

package org.elasticsearch.common.inject.internal;

import java.util.Timer;


class ExpirationTimer {
    static Timer instance = new Timer(true);
}
