
package hudson.diagnosis;

import hudson.Extension;
import hudson.PluginWrapper;
import hudson.init.Initializer;
import hudson.model.AdministrativeMonitor;
import hudson.model.Descriptor;
import jenkins.model.Jenkins;
import org.jenkinsci.Symbol;

import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

import static hudson.init.InitMilestone.EXTENSIONS_AUGMENTED;


@Extension @Symbol("nullId")
public class NullIdDescriptorMonitor extends AdministrativeMonitor {

    @Override
    public String getDisplayName() {
        return Messages.NullIdDescriptorMonitor_DisplayName();
    }

    private final List<Descriptor> problems = new ArrayList<Descriptor>();

    @Override
    public boolean isActivated() {
        return !problems.isEmpty();
    }

    public List<Descriptor> getProblems() {
        return Collections.unmodifiableList(problems);
    }

    @Initializer(after=EXTENSIONS_AUGMENTED)
    public void verify() {
        Jenkins h = Jenkins.getInstance();
        for (Descriptor d : h.getExtensionList(Descriptor.class)) {
            PluginWrapper p = h.getPluginManager().whichPlugin(d.getClass());
            String id;
            try {
                id = d.getId();
            } catch (Throwable t) {
                LOGGER.log(Level.SEVERE,MessageFormat.format("Descriptor {0} from plugin {1} with display name {2} reported an exception for ID",
                        d, p == null ? "???" : p.getLongName(), d.getDisplayName()),t);
                problems.add(d);
                continue;
            }
            if (id==null) {
                LOGGER.severe(MessageFormat.format("Descriptor {0} from plugin {1} with display name {2} has null ID",
                        d, p==null?"???":p.getLongName(), d.getDisplayName()));
                problems.add(d);
            }
        }
    }

    private static final Logger LOGGER = Logger.getLogger(NullIdDescriptorMonitor.class.getName());
}
