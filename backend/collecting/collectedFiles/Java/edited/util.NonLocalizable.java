package jenkins.util;

import org.jvnet.localizer.Localizable;

import java.util.Locale;


public class NonLocalizable extends Localizable {
    
    private final String nonLocalizable;

    
    public NonLocalizable(String nonLocalizable) {
        super(null, null);
        this.nonLocalizable = nonLocalizable;
    }

    @Override
    public String toString(Locale locale) {
        return nonLocalizable;
    }

    @Override
    public String toString() {
        return nonLocalizable;
    }
}
