package org.elasticsearch.ingest.attachment;



import org.apache.tika.Tika;
import org.apache.tika.exception.TikaException;
import org.apache.tika.metadata.Metadata;
import org.apache.tika.mime.MediaType;
import org.apache.tika.parser.AutoDetectParser;
import org.apache.tika.parser.Parser;
import org.apache.tika.parser.ParserDecorator;
import org.elasticsearch.SpecialPermission;
import org.elasticsearch.bootstrap.FilePermissionUtils;
import org.elasticsearch.bootstrap.JarHell;
import org.elasticsearch.bootstrap.JavaVersion;
import org.elasticsearch.common.SuppressForbidden;
import org.elasticsearch.common.io.PathUtils;

import java.io.ByteArrayInputStream;
import java.io.IOException;
import java.io.UncheckedIOException;
import java.lang.reflect.ReflectPermission;
import java.net.URISyntaxException;
import java.net.URL;
import java.net.URLClassLoader;
import java.nio.file.Files;
import java.nio.file.Path;
import java.security.AccessControlContext;
import java.security.AccessController;
import java.security.PermissionCollection;
import java.security.Permissions;
import java.security.PrivilegedActionException;
import java.security.PrivilegedExceptionAction;
import java.security.ProtectionDomain;
import java.security.SecurityPermission;
import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.PropertyPermission;
import java.util.Set;


final class TikaImpl {

    
    private static final Set<MediaType> EXCLUDES = new HashSet<>(Arrays.asList(
        MediaType.application("vnd.ms-visio.drawing"),
        MediaType.application("vnd.ms-visio.drawing.macroenabled.12"),
        MediaType.application("vnd.ms-visio.stencil"),
        MediaType.application("vnd.ms-visio.stencil.macroenabled.12"),
        MediaType.application("vnd.ms-visio.template"),
        MediaType.application("vnd.ms-visio.template.macroenabled.12"),
        MediaType.application("vnd.ms-visio.drawing")
    ));

    
    private static final Parser PARSERS[] = new Parser[] {
                new org.apache.tika.parser.html.HtmlParser(),
        new org.apache.tika.parser.rtf.RTFParser(),
        new org.apache.tika.parser.pdf.PDFParser(),
        new org.apache.tika.parser.txt.TXTParser(),
        new org.apache.tika.parser.microsoft.OfficeParser(),
        new org.apache.tika.parser.microsoft.OldExcelParser(),
        ParserDecorator.withoutTypes(new org.apache.tika.parser.microsoft.ooxml.OOXMLParser(), EXCLUDES),
        new org.apache.tika.parser.odf.OpenDocumentParser(),
        new org.apache.tika.parser.iwork.IWorkPackageParser(),
        new org.apache.tika.parser.xml.DcXMLParser(),
        new org.apache.tika.parser.epub.EpubParser(),
    };

    
    private static final AutoDetectParser PARSER_INSTANCE = new AutoDetectParser(PARSERS);

    
    private static final Tika TIKA_INSTANCE = new Tika(PARSER_INSTANCE.getDetector(), PARSER_INSTANCE);

    
    static String parse(final byte content[], final Metadata metadata, final int limit) throws TikaException, IOException {
                SpecialPermission.check();

        try {
            return AccessController.doPrivileged((PrivilegedExceptionAction<String>)
                () -> TIKA_INSTANCE.parseToString(new ByteArrayInputStream(content), metadata, limit), RESTRICTED_CONTEXT);
        } catch (PrivilegedActionException e) {
                        Throwable cause = e.getCause();
            if (cause instanceof TikaException) {
                throw (TikaException) cause;
            } else if (cause instanceof IOException) {
                throw (IOException) cause;
            } else {
                throw new AssertionError(cause);
            }
        }
    }

            private static final AccessControlContext RESTRICTED_CONTEXT = new AccessControlContext(
        new ProtectionDomain[] {
            new ProtectionDomain(null, getRestrictedPermissions())
        }
    );

            @SuppressForbidden(reason = "adds access to tmp directory")
    static PermissionCollection getRestrictedPermissions() {
        Permissions perms = new Permissions();
                perms.add(new PropertyPermission("*", "read"));
        perms.add(new RuntimePermission("getenv.TIKA_CONFIG"));

        try {
                                    addReadPermissions(perms, JarHell.parseClassPath());
                        if (TikaImpl.class.getClassLoader() instanceof URLClassLoader) {
                URL[] urls = ((URLClassLoader)TikaImpl.class.getClassLoader()).getURLs();
                Set<URL> set = new LinkedHashSet<>(Arrays.asList(urls));
                if (set.size() != urls.length) {
                    throw new AssertionError("duplicate jars: " + Arrays.toString(urls));
                }
                addReadPermissions(perms, set);
            }
                        FilePermissionUtils.addDirectoryPath(perms, "java.io.tmpdir",
                PathUtils.get(System.getProperty("java.io.tmpdir")), "read,readlink,write,delete");
        } catch (IOException e) {
            throw new UncheckedIOException(e);
        }
                perms.add(new SecurityPermission("putProviderProperty.BC"));
        perms.add(new SecurityPermission("insertProvider"));
        perms.add(new ReflectPermission("suppressAccessChecks"));
                perms.add(new RuntimePermission("getClassLoader"));
                if (JavaVersion.current().compareTo(JavaVersion.parse("10")) >= 0) {
            
            assert JavaVersion.current().compareTo(JavaVersion.parse("11")) < 0;
            perms.add(new RuntimePermission("accessDeclaredMembers"));
        }
        perms.setReadOnly();
        return perms;
    }

        @SuppressForbidden(reason = "adds access to jar resources")
    static void addReadPermissions(Permissions perms, Set<URL> resources) throws IOException {
        try {
            for (URL url : resources) {
                Path path = PathUtils.get(url.toURI());
                if (Files.isDirectory(path)) {
                    FilePermissionUtils.addDirectoryPath(perms, "class.path", path, "read,readlink");
                } else {
                    FilePermissionUtils.addSingleFilePath(perms, path, "read,readlink");
                }
            }
        } catch (URISyntaxException bogus) {
            throw new RuntimeException(bogus);
        }
    }
}
