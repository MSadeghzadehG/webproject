

package org.elasticsearch.rest;

import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.Set;


final class MethodHandlers {

    private final String path;
    private final Map<RestRequest.Method, RestHandler> methodHandlers;

    MethodHandlers(String path, RestHandler handler, RestRequest.Method... methods) {
        this.path = path;
        this.methodHandlers = new HashMap<>(methods.length);
        for (RestRequest.Method method : methods) {
            methodHandlers.put(method, handler);
        }
    }

    
    public MethodHandlers addMethod(RestRequest.Method method, RestHandler handler) {
        RestHandler existing = methodHandlers.putIfAbsent(method, handler);
        if (existing != null) {
            throw new IllegalArgumentException("Cannot replace existing handler for [" + path + "] for method: " + method);
        }
        return this;
    }

    
    public MethodHandlers addMethods(RestHandler handler, RestRequest.Method... methods) {
        for (RestRequest.Method method : methods) {
            addMethod(method, handler);
        }
        return this;
    }

    
    public Optional<RestHandler> getHandler(RestRequest.Method method) {
        return Optional.ofNullable(methodHandlers.get(method));
    }

    
    public Set<RestRequest.Method> getValidMethods() {
        return methodHandlers.keySet();
    }
}
