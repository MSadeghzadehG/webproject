

package org.elasticsearch.common.inject;

import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.spi.Message;

import java.util.Collection;
import java.util.Locale;
import java.util.Set;

import static java.util.Collections.unmodifiableSet;
import static org.elasticsearch.common.util.set.Sets.newHashSet;


public final class ConfigurationException extends RuntimeException {
    private final Set<Message> messages;
    private Object partialValue = null;

    
    public ConfigurationException(Iterable<Message> messages) {
        this.messages = unmodifiableSet(newHashSet(messages));
        initCause(Errors.getOnlyCause(this.messages));
    }

    
    public ConfigurationException withPartialValue(Object partialValue) {
        if (this.partialValue != null) {
            String message = String.format(Locale.ROOT, "Can't clobber existing partial value %s with %s", this.partialValue, partialValue);
            throw new IllegalStateException(message);
        }
        ConfigurationException result = new ConfigurationException(messages);
        result.partialValue = partialValue;
        return result;
    }

    
    public Collection<Message> getErrorMessages() {
        return messages;
    }

    
    @SuppressWarnings("unchecked")     public <E> E getPartialValue() {
        return (E) partialValue;
    }

    @Override
    public String getMessage() {
        return Errors.format("Guice configuration errors", messages);
    }
}