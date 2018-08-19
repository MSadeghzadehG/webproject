

package org.elasticsearch.painless.node;

import org.elasticsearch.painless.Globals;
import org.elasticsearch.painless.Locals;
import org.elasticsearch.painless.Location;
import org.elasticsearch.painless.MethodWriter;

import java.util.Collections;
import java.util.List;
import java.util.Set;

import static java.util.Collections.emptyList;


public final class SDeclBlock extends AStatement {

    private final List<SDeclaration> declarations;

    public SDeclBlock(Location location, List<SDeclaration> declarations) {
        super(location);

        this.declarations = Collections.unmodifiableList(declarations);
    }

    @Override
    void extractVariables(Set<String> variables) {
        for (SDeclaration declaration : declarations) {
            declaration.extractVariables(variables);
        }
    }

    @Override
    void analyze(Locals locals) {
        for (SDeclaration declaration : declarations) {
            declaration.analyze(locals);
        }

        statementCount = declarations.size();
    }

    @Override
    void write(MethodWriter writer, Globals globals) {
        for (AStatement declaration : declarations) {
            declaration.write(writer, globals);
        }
    }

    @Override
    public String toString() {
        return multilineToString(emptyList(), declarations);
    }
}
