
package hudson.cli;

import hudson.Extension;
import hudson.model.Run;

import java.io.IOException;
import java.io.PrintStream;
import java.util.HashSet;
import java.util.List;
import org.kohsuke.accmod.Restricted;
import org.kohsuke.accmod.restrictions.NoExternalUse;


@Restricted(NoExternalUse.class) @Extension
public class DeleteBuildsCommand extends RunRangeCommand {
    @Override
    public String getShortDescription() {
        return Messages.DeleteBuildsCommand_ShortDescription();
    }

    @Override
    protected void printUsageSummary(PrintStream stderr) {
        stderr.println(
            "Delete build records of a specified job, possibly in a bulk. "
        );
    }

    @Override
    protected int act(List<Run<?, ?>> builds) throws IOException {
        job.checkPermission(Run.DELETE);

        final HashSet<Integer> hsBuilds = new HashSet<Integer>();

        for (Run<?, ?> build : builds) {
            if (!hsBuilds.contains(build.number)) {
                build.delete();
                hsBuilds.add(build.number);
            }
        }

        stdout.println("Deleted "+hsBuilds.size()+" builds");

        return 0;
    }

}
