package hudson.util;

import java.io.File;
import java.io.FileFilter;
import java.io.IOException;
import java.io.Serializable;


public abstract class FileVisitor {
    
    public abstract void visit(File f, String relativePath) throws IOException;

    
    public void visitSymlink(File link, String target, String relativePath) throws IOException {
        visit(link,relativePath);
    }

    
    public boolean understandsSymlink() {
        return false;
    }
    
    
    public final FileVisitor with(FileFilter f) {
        if(f==null) return this;
        return new FilterFileVisitor(f,this);
    }

    private static final class FilterFileVisitor extends FileVisitor implements Serializable {
        private final FileFilter filter;
        private final FileVisitor visitor;

        private FilterFileVisitor(FileFilter filter, FileVisitor visitor) {
            this.filter = filter!=null ? filter : PASS_THROUGH;
            this.visitor = visitor;
        }

        public void visit(File f, String relativePath) throws IOException {
            if(f.isDirectory() || filter.accept(f))
                visitor.visit(f,relativePath);
        }

        private static final FileFilter PASS_THROUGH = new FileFilter() {
            public boolean accept(File pathname) {
                return true;
            }
        };

        private static final long serialVersionUID = 1L;
    }
}
