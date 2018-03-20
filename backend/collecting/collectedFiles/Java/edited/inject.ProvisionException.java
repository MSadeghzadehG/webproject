

package org.elasticsearch.common.inject;

import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.spi.Message;

import java.util.Collection;
import java.util.Collections;
import java.util.Set;

import static java.util.Collections.singleton;
import static java.util.Collections.unmodifiableSet;
import static org.elasticsearch.common.util.set.Sets.newHashSet;


public final class ProvisionException extends RuntimeException {
    private final Set<Message> messages;

    
    public ProvisionException(Iterable<Message> messages) {
        this.messages = unmodifiableSet(newHashSet(messages));
        if (this.messages.isEmpty()) {
            throw new IllegalArgumentException();
        }
        initCause(Errors.getOnlyCause(this.messages));
    }

    public ProvisionException(String message, Throwable cause) {
        super(cause);
        this.messages = singleton(new Message(Collections.emptyList(), message, cause));
    }

    public ProvisionException(String message) {
        this.messages = singleton(new Message(message));
    }

    
    public Collection<Message> getErrorMessages() {
        return messages;
    }

    @Override
    public String getMessage() {
        return Errors.format("Guice provision errors", messages);
    }
}
