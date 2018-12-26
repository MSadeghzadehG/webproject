

package java.security;

public class AccessControlException extends SecurityException {
	private final Permission permission;

	public AccessControlException (String message) {
		this(message, null);
	}

	public AccessControlException (String message, Permission permission) {
		super(message);
		this.permission = permission;
	}

	public Permission getPermission () {
		return permission;
	}
}
