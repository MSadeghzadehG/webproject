package jenkins.diagnosis;

import com.sun.akuma.JavaVMArguments;
import hudson.Extension;
import hudson.Functions;
import hudson.Util;
import hudson.model.AdministrativeMonitor;
import hudson.util.jna.Kernel32Utils;
import java.nio.file.Files;
import java.nio.file.InvalidPathException;
import java.nio.file.OpenOption;
import java.nio.file.StandardOpenOption;
import jenkins.model.Jenkins;
import org.apache.tools.ant.DirectoryScanner;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.types.FileSet;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.io.Reader;
import java.nio.MappedByteBuffer;
import java.nio.channels.FileChannel;
import java.nio.channels.FileChannel.MapMode;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.commons.io.IOUtils;
import org.jenkinsci.Symbol;


@Extension(optional=true) @Symbol("hsErrPid")
public class HsErrPidList extends AdministrativeMonitor {
    
     final List<HsErrPidFile> files = new ArrayList<HsErrPidFile>();

    
    private MappedByteBuffer map;

    public HsErrPidList() {
        if (Functions.getIsUnitTest()) {
            return;
        }
        try {
            try (FileChannel ch = FileChannel.open(getSecretKeyFile().toPath(), StandardOpenOption.READ)) {
                map = ch.map(MapMode.READ_ONLY,0,1);
            } catch (InvalidPathException e) {
                throw new IOException(e);
            }
                
            scan("./hs_err_pid%p.log");
            if (Functions.isWindows()) {
                File dir = Kernel32Utils.getTempDir();
                if (dir!=null) {
                    scan(dir.getPath() + "\\hs_err_pid%p.log");
                }
            } else {
                scan("/tmp/hs_err_pid%p.log");
            }
            
                        JavaVMArguments args = JavaVMArguments.current();
            for (String a : args) {
                                if (a.startsWith(ERROR_FILE_OPTION)) {
                    scan(a.substring(ERROR_FILE_OPTION.length()));
                }
            }
        } catch (UnsupportedOperationException e) {
                    } catch (Throwable e) {
            LOGGER.log(Level.WARNING, "Failed to list up hs_err_pid files", e);
        }
    }

    @Override
    public String getDisplayName() {
        return "JVM Crash Reports";
    }

    
    public List<HsErrPidFile> getFiles() {
        return files;
    }


    private void scan(String pattern) {
        LOGGER.fine("Scanning "+pattern+" for hs_err_pid files");

        pattern = pattern.replace("%p","*").replace("%%","%");
        File f = new File(pattern).getAbsoluteFile();
        if (!pattern.contains("*"))
            scanFile(f);
        else {            File commonParent = f;
            while (commonParent!=null && commonParent.getPath().contains("*")) {
                commonParent = commonParent.getParentFile();
            }
            if (commonParent==null) {
                LOGGER.warning("Failed to process "+f);
                return;             }

            FileSet fs = Util.createFileSet(commonParent, f.getPath().substring(commonParent.getPath().length()+1), null);
            DirectoryScanner ds = fs.getDirectoryScanner(new Project());
            for (String child : ds.getIncludedFiles()) {
                scanFile(new File(commonParent,child));
            }
        }
    }

    private void scanFile(File log) {
        LOGGER.fine("Scanning "+log);

        try (Reader rawReader = new FileReader(log);
             BufferedReader r = new BufferedReader(rawReader)) {

            if (!findHeader(r))
                return;

                        String secretKey = getSecretKeyFile().getAbsolutePath();


            String line;
            while ((line=r.readLine())!=null) {
                if (line.contains(secretKey)) {
                    files.add(new HsErrPidFile(this,log));
                    return;
                }
            }
        } catch (IOException e) {
                        LOGGER.log(Level.FINE, "Failed to parse hs_err_pid file: " + log, e);
        }
    }

    private File getSecretKeyFile() {
        return new File(Jenkins.getInstance().getRootDir(),"secret.key");
    }

    private boolean findHeader(BufferedReader r) throws IOException {
        for (int i=0; i<5; i++) {
            String line = r.readLine();
            if (line==null)
                return false;
            if (line.startsWith("# A fatal error has been detected by the Java Runtime Environment:"))
                return true;
        }
        return false;
    }

    @Override
    public boolean isActivated() {
        return !files.isEmpty();
    }

    private static final String ERROR_FILE_OPTION = "-XX:ErrorFile=";
    private static final Logger LOGGER = Logger.getLogger(HsErrPidList.class.getName());
}
