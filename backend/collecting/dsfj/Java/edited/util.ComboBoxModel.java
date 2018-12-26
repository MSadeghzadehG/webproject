
package hudson.util;

import net.sf.json.JSONArray;
import org.kohsuke.stapler.HttpResponse;
import org.kohsuke.stapler.StaplerRequest;
import org.kohsuke.stapler.StaplerResponse;
import org.kohsuke.stapler.export.Flavor;

import javax.servlet.ServletException;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Collection;

import static java.util.Arrays.asList;


public class ComboBoxModel extends ArrayList<String> implements HttpResponse {
    public ComboBoxModel(int initialCapacity) {
        super(initialCapacity);
    }

    public ComboBoxModel() {
    }

    public ComboBoxModel(Collection<? extends String> c) {
        super(c);
    }

    public ComboBoxModel(String... values) {
        this(asList(values));
    }

    public void generateResponse(StaplerRequest req, StaplerResponse rsp, Object node) throws IOException, ServletException {
        rsp.setContentType(Flavor.JSON.contentType);
        PrintWriter w = rsp.getWriter();
        JSONArray.fromObject(this).write(w);
    }
}
