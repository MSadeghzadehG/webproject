

package org.elasticsearch.common.inject.internal;

import java.lang.reflect.Constructor;
import java.lang.reflect.Member;


public class StackTraceElements {

    public static Object forMember(Member member) {
        if (member == null) {
            return SourceProvider.UNKNOWN_SOURCE;
        }

        Class declaringClass = member.getDeclaringClass();

        String fileName = null;
        int lineNumber = -1;

        Class<? extends Member> memberType = MoreTypes.memberType(member);
        String memberName = memberType == Constructor.class ? "<init>" : member.getName();
        return new StackTraceElement(declaringClass.getName(), memberName, fileName, lineNumber);
    }

    public static Object forType(Class<?> implementation) {
        String fileName = null;
        int lineNumber = -1;

        return new StackTraceElement(implementation.getName(), "class", fileName, lineNumber);
    }
}
