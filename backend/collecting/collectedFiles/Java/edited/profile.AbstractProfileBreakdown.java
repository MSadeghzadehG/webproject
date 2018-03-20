

package org.elasticsearch.search.profile;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;


public abstract class AbstractProfileBreakdown<T extends Enum<T>> {

    
    private final Timer[] timings;
    private final T[] timingTypes;

    
    public AbstractProfileBreakdown(Class<T> clazz) {
        this.timingTypes = clazz.getEnumConstants();
        timings = new Timer[timingTypes.length];
        for (int i = 0; i < timings.length; ++i) {
            timings[i] = new Timer();
        }
    }

    public Timer getTimer(T timing) {
        return timings[timing.ordinal()];
    }

    
    public Map<String, Long> toTimingMap() {
        Map<String, Long> map = new HashMap<>();
        for (T timingType : timingTypes) {
            map.put(timingType.toString(), timings[timingType.ordinal()].getApproximateTiming());
            map.put(timingType.toString() + "_count", timings[timingType.ordinal()].getCount());
        }
        return Collections.unmodifiableMap(map);
    }
}
