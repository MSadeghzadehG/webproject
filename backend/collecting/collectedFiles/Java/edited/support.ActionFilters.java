

package org.elasticsearch.action.support;

import java.util.Arrays;
import java.util.Comparator;
import java.util.Set;


public class ActionFilters {

    private final ActionFilter[] filters;

    public ActionFilters(Set<ActionFilter> actionFilters) {
        this.filters = actionFilters.toArray(new ActionFilter[actionFilters.size()]);
        Arrays.sort(filters, new Comparator<ActionFilter>() {
            @Override
            public int compare(ActionFilter o1, ActionFilter o2) {
                return Integer.compare(o1.order(), o2.order());
            }
        });
    }

    
    public ActionFilter[] filters() {
        return filters;
    }
}
