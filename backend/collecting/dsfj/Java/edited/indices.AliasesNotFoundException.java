
package org.elasticsearch.rest.action.admin.indices;

import org.elasticsearch.ResourceNotFoundException;
import org.elasticsearch.common.io.stream.StreamInput;

import java.io.IOException;
import java.util.Arrays;

public class AliasesNotFoundException extends ResourceNotFoundException {

    public AliasesNotFoundException(String... names) {
        super("aliases " +  Arrays.toString(names) + " missing");
        this.setResources("aliases", names);
    }

    public AliasesNotFoundException(StreamInput in) throws IOException{
        super(in);
    }
}
