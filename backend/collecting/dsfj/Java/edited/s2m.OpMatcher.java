package jenkins.security.s2m;

import jenkins.ReflectiveFilePathFilter;

import java.io.File;


interface OpMatcher {
    boolean matches(String op);

    OpMatcher ALL = new OpMatcher() {
        @Override
        public boolean matches(String op) {
            return true;
        }
    };
}
