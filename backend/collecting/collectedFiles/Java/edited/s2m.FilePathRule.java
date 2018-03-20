package jenkins.security.s2m;

import hudson.FilePath;

import java.util.regex.Pattern;


class FilePathRule {
    final Pattern path;
    final OpMatcher op;
    final boolean allow;

    FilePathRule(Pattern path, OpMatcher op, boolean allow) {
        this.path = path;
        this.op = op;
        this.allow = allow;
    }
}
