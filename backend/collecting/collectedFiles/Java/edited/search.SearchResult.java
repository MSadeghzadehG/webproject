package hudson.search;

import java.util.List;


public interface SearchResult extends List<SuggestedItem> {

    boolean hasMoreResults();

}
