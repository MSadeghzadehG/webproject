

package org.elasticsearch.common.inject;

import org.elasticsearch.common.inject.internal.Errors;
import org.elasticsearch.common.inject.spi.PrivateElements;

import java.util.ArrayList;
import java.util.List;


class PrivateElementProcessor extends AbstractProcessor {

    private final Stage stage;
    private final List<InjectorShell.Builder> injectorShellBuilders = new ArrayList<>();

    PrivateElementProcessor(Errors errors, Stage stage) {
        super(errors);
        this.stage = stage;
    }

    @Override
    public Boolean visit(PrivateElements privateElements) {
        InjectorShell.Builder builder = new InjectorShell.Builder()
                .parent(injector)
                .stage(stage)
                .privateElements(privateElements);
        injectorShellBuilders.add(builder);
        return true;
    }

    public List<InjectorShell.Builder> getInjectorShellBuilders() {
        return injectorShellBuilders;
    }
}
