

package org.elasticsearch.common.inject;

import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.internal.ErrorsException;
import org.elasticsearch.common.inject.internal.FailableCache;
import org.elasticsearch.common.inject.spi.InjectionPoint;
import org.elasticsearch.common.inject.spi.TypeListenerBinding;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;


class MembersInjectorStore {
    private final InjectorImpl injector;
    private final List<TypeListenerBinding> typeListenerBindings;

    private final FailableCache<TypeLiteral<?>, MembersInjectorImpl<?>> cache
            = new FailableCache<TypeLiteral<?>, MembersInjectorImpl<?>>() {
        @Override
        protected MembersInjectorImpl<?> create(TypeLiteral<?> type, Errors errors)
                throws ErrorsException {
            return createWithListeners(type, errors);
        }
    };

    MembersInjectorStore(InjectorImpl injector,
                         List<TypeListenerBinding> typeListenerBindings) {
        this.injector = injector;
        this.typeListenerBindings = Collections.unmodifiableList(typeListenerBindings);
    }

    
    public boolean hasTypeListeners() {
        return !typeListenerBindings.isEmpty();
    }

    
    @SuppressWarnings("unchecked")     public <T> MembersInjectorImpl<T> get(TypeLiteral<T> key, Errors errors) throws ErrorsException {
        return (MembersInjectorImpl<T>) cache.get(key, errors);
    }

    
    private <T> MembersInjectorImpl<T> createWithListeners(TypeLiteral<T> type, Errors errors)
            throws ErrorsException {
        int numErrorsBefore = errors.size();

        Set<InjectionPoint> injectionPoints;
        try {
            injectionPoints = InjectionPoint.forInstanceMethodsAndFields(type);
        } catch (ConfigurationException e) {
            errors.merge(e.getErrorMessages());
            injectionPoints = e.getPartialValue();
        }
        List<SingleMemberInjector> injectors = getInjectors(injectionPoints, errors);
        errors.throwIfNewErrors(numErrorsBefore);

        EncounterImpl<T> encounter = new EncounterImpl<>(errors, injector.lookups);
        for (TypeListenerBinding typeListener : typeListenerBindings) {
            if (typeListener.getTypeMatcher().matches(type)) {
                try {
                    typeListener.getListener().hear(type, encounter);
                } catch (RuntimeException e) {
                    errors.errorNotifyingTypeListener(typeListener, type, e);
                }
            }
        }
        encounter.invalidate();
        errors.throwIfNewErrors(numErrorsBefore);

        return new MembersInjectorImpl<>(injector, type, encounter, injectors);
    }

    
    List<SingleMemberInjector> getInjectors(
            Set<InjectionPoint> injectionPoints, Errors errors) {
        List<SingleMemberInjector> injectors = new ArrayList<>();
        for (InjectionPoint injectionPoint : injectionPoints) {
            try {
                Errors errorsForMember = injectionPoint.isOptional()
                        ? new Errors(injectionPoint)
                        : errors.withSource(injectionPoint);
                SingleMemberInjector injector = injectionPoint.getMember() instanceof Field
                        ? new SingleFieldInjector(this.injector, injectionPoint, errorsForMember)
                        : new SingleMethodInjector(this.injector, injectionPoint, errorsForMember);
                injectors.add(injector);
            } catch (ErrorsException ignoredForNow) {
                            }
        }
        return Collections.unmodifiableList(injectors);
    }
}
