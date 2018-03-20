

package org.elasticsearch.common.inject;

import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.internal.ErrorsException;
import org.elasticsearch.common.inject.internal.InternalContext;
import org.elasticsearch.common.inject.spi.InjectionPoint;
import org.elasticsearch.common.inject.spi.InjectionRequest;
import org.elasticsearch.common.inject.spi.StaticInjectionRequest;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;


class InjectionRequestProcessor extends AbstractProcessor {

    private final List<StaticInjection> staticInjections = new ArrayList<>();
    private final Initializer initializer;

    InjectionRequestProcessor(Errors errors, Initializer initializer) {
        super(errors);
        this.initializer = initializer;
    }

    @Override
    public Boolean visit(StaticInjectionRequest request) {
        staticInjections.add(new StaticInjection(injector, request));
        return true;
    }

    @Override
    public Boolean visit(InjectionRequest request) {
        Set<InjectionPoint> injectionPoints;
        try {
            injectionPoints = request.getInjectionPoints();
        } catch (ConfigurationException e) {
            errors.merge(e.getErrorMessages());
            injectionPoints = e.getPartialValue();
        }

        initializer.requestInjection(
                injector, request.getInstance(), request.getSource(), injectionPoints);
        return true;
    }

    public void validate() {
        for (StaticInjection staticInjection : staticInjections) {
            staticInjection.validate();
        }
    }

    public void injectMembers() {
        for (StaticInjection staticInjection : staticInjections) {
            staticInjection.injectMembers();
        }
    }

    
    private class StaticInjection {
        final InjectorImpl injector;
        final Object source;
        final StaticInjectionRequest request;
        List<SingleMemberInjector> memberInjectors;

        StaticInjection(InjectorImpl injector, StaticInjectionRequest request) {
            this.injector = injector;
            this.source = request.getSource();
            this.request = request;
        }

        void validate() {
            Errors errorsForMember = errors.withSource(source);
            Set<InjectionPoint> injectionPoints;
            try {
                injectionPoints = request.getInjectionPoints();
            } catch (ConfigurationException e) {
                errors.merge(e.getErrorMessages());
                injectionPoints = e.getPartialValue();
            }
            memberInjectors = injector.membersInjectorStore.getInjectors(
                    injectionPoints, errorsForMember);
        }

        void injectMembers() {
            try {
                injector.callInContext(new ContextualCallable<Void>() {
                    @Override
                    public Void call(InternalContext context) {
                        for (SingleMemberInjector injector : memberInjectors) {
                            injector.inject(errors, context, null);
                        }
                        return null;
                    }
                });
            } catch (ErrorsException e) {
                throw new AssertionError();
            }
        }
    }
}
