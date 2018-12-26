

package java.security;

public abstract class Permission {

	protected String name;

	public Permission (String name) {
		this.name = name;
	}

	public String getName () {
		return name;
	}

	@Override
	public String toString () {
		return this.getClass().getName() + '[' + name + ']';
	}

	public PermissionCollection newPermissionCollection () {
		return null;
	}
}
