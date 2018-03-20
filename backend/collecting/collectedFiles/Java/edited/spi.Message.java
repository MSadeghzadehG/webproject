

package org.elasticsearch.common.inject.spi;

import org.elasticsearch.common.inject.Binder;
import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.internal.SourceProvider;

import java.util.Collections;
import java.util.List;
import java.util.Objects;


public final class Message implements Element {
    private final String message;
    private final Throwable cause;
    private final List<Object> sources;

    
    public Message(List<Object> sources, String message, Throwable cause) {
        this.sources = Collections.unmodifiableList(sources);
        this.message = Objects.requireNonNull(message, "message");
        this.cause = cause;
    }

    public Message(Object source, String message) {
        this(Collections.singletonList(source), message, null);
    }

    public Message(Object source, Throwable cause) {
        this(Collections.singletonList(source), null, cause);
    }

    public Message(String message) {
        this(Collections.emptyList(), message, null);
    }

    @Override
    public String getSource() {
        return sources.isEmpty()
                ? SourceProvider.UNKNOWN_SOURCE.toString()
                : Errors.convert(sources.get(sources.size() - 1)).toString();
    }

    
    public List<Object> getSources() {
        return sources;
    }

    
    public String getMessage() {
        return message;
    }

    
    @Override
    public <T> T acceptVisitor(ElementVisitor<T> visitor) {
        return visitor.visit(this);
    }

    
    public Throwable getCause() {
        return cause;
    }

    @Override
    public String toString() {
        return message;
    }

    @Override
    public int hashCode() {
        return message.hashCode();
    }

    @Override
    public boolean equals(Object o) {
        if (!(o instanceof Message)) {
            return false;
        }
        Message e = (Message) o;
        return message.equals(e.message) && Objects.equals(cause, e.cause) && sources.equals(e.sources);
    }

    
    @Override
    public void applyTo(Binder binder) {
        binder.withSource(getSource()).addError(this);
    }
}