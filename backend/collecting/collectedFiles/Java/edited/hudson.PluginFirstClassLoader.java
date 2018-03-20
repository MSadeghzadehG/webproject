
package hudson;

import java.io.Closeable;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Enumeration;
import java.util.List;

import jenkins.util.AntWithFindResourceClassLoader;


public class PluginFirstClassLoader
    extends AntWithFindResourceClassLoader
    implements Closeable
{

    public PluginFirstClassLoader() {
        super(null, false);
    }

    private List<URL> urls = new ArrayList<URL>();

    public void addPathFiles( Collection<File> paths )
        throws IOException
    {
        for ( File f : paths )
        {
            urls.add( f.toURI().toURL() );
            addPathFile( f );
        }
    }

    
    public List<URL> getURLs() 
    {
        return urls;
    }
    
    public void close()
        throws IOException
    {
        cleanup();
    }

    @Override
    protected Enumeration findResources( String arg0, boolean arg1 )
        throws IOException
    {
        Enumeration enu = super.findResources( arg0, arg1 );
        return enu;
    }

    @Override
    protected Enumeration findResources( String name )
        throws IOException
    {
        Enumeration enu = super.findResources( name );
        return enu;
    }

    @Override
    public URL getResource( String arg0 )
    {
        URL url = super.getResource( arg0 );
        return url;
    }

    @Override
    public InputStream getResourceAsStream( String name )
    {
        InputStream is = super.getResourceAsStream( name );
        return is;
    }
}
