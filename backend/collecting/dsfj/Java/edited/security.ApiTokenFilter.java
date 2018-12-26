package jenkins.security;

import javax.servlet.Filter;
import java.util.Collections;
import java.util.List;


@Deprecated
public class ApiTokenFilter extends BasicHeaderProcessor {
    @Override
    protected List<? extends BasicHeaderAuthenticator> all() {
        return Collections.singletonList(new BasicHeaderApiTokenAuthenticator());
    }
}
