

package org.elasticsearch.index.query.support;

import org.elasticsearch.index.mapper.ObjectMapper;

import java.util.Deque;
import java.util.LinkedList;


public final class NestedScope {

    private final Deque<ObjectMapper> levelStack = new LinkedList<>();

    
    public ObjectMapper getObjectMapper() {
        return levelStack.peek();
    }

    
    public ObjectMapper nextLevel(ObjectMapper level) {
        ObjectMapper previous = levelStack.peek();
        levelStack.push(level);
        return previous;
    }

    
    public ObjectMapper previousLevel() {
        return levelStack.pop();
    }

}
