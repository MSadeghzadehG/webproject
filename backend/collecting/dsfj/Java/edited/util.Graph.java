
package hudson.util;

import org.jfree.chart.JFreeChart;
import org.jfree.chart.ChartRenderingInfo;
import org.jfree.chart.ChartUtilities;
import org.jfree.chart.plot.Plot;
import org.kohsuke.stapler.StaplerRequest;
import org.kohsuke.stapler.StaplerResponse;

import javax.servlet.ServletOutputStream;
import javax.imageio.ImageIO;
import java.io.IOException;
import java.util.Calendar;
import java.awt.image.BufferedImage;
import java.awt.*;
import javax.annotation.Nonnull;
import javax.annotation.CheckForNull;


public abstract class Graph {
    private final long timestamp;
    private final int defaultW;
    private final int defaultH;
    private volatile JFreeChart graph;

    
    protected Graph(long timestamp, int defaultW, int defaultH) {
        this.timestamp = timestamp;
        this.defaultW = defaultW;
        this.defaultH = defaultH;
    }

    protected Graph(Calendar timestamp, int defaultW, int defaultH) {
        this(timestamp.getTimeInMillis(),defaultW,defaultH);
    }

    
    protected abstract JFreeChart createGraph();

    private BufferedImage render(StaplerRequest req, ChartRenderingInfo info) {
        String w = req.getParameter("width");
        if(w==null)     w=String.valueOf(defaultW);
        String h = req.getParameter("height");
        if(h==null)     h=String.valueOf(defaultH);

        Color graphBg = stringToColor(req.getParameter("graphBg"));
        Color plotBg = stringToColor(req.getParameter("plotBg"));

        if (graph==null)    graph = createGraph();
        graph.setBackgroundPaint(graphBg);
        Plot p = graph.getPlot();
        p.setBackgroundPaint(plotBg);

        return graph.createBufferedImage(Integer.parseInt(w),Integer.parseInt(h),info);
    }

    @Nonnull private static Color stringToColor(@CheckForNull String s) {
        if (s != null) {
            try {
                return Color.decode("0x" + s);
            } catch (NumberFormatException e) {
                return Color.WHITE;
            }
        } else {
            return Color.WHITE;
        }
    }

    
    public void doPng(StaplerRequest req, StaplerResponse rsp) throws IOException {
        if (req.checkIfModified(timestamp, rsp)) return;

        try {
            BufferedImage image = render(req,null);
            rsp.setContentType("image/png");
            ServletOutputStream os = rsp.getOutputStream();
            ImageIO.write(image, "PNG", os);
            os.close();
        } catch(Error e) {
            
            if(e.getMessage().contains("Probable fatal error:No fonts found")) {
                rsp.sendRedirect2(req.getContextPath()+"/images/headless.png");
                return;
            }
            throw e;         } catch(HeadlessException e) {
                        rsp.sendRedirect2(req.getContextPath()+"/images/headless.png");
        }
    }

    
    public void doMap(StaplerRequest req, StaplerResponse rsp) throws IOException {
        if (req.checkIfModified(timestamp, rsp)) return;

        ChartRenderingInfo info = new ChartRenderingInfo();
        render(req,info);

        rsp.setContentType("text/plain;charset=UTF-8");
        rsp.getWriter().println(ChartUtilities.getImageMap( "map", info ));
    }
}
