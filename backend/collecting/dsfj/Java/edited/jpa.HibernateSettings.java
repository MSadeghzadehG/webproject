

package org.springframework.boot.autoconfigure.orm.jpa;

import java.util.ArrayList;
import java.util.Collection;
import java.util.function.Supplier;

import org.hibernate.boot.model.naming.ImplicitNamingStrategy;
import org.hibernate.boot.model.naming.PhysicalNamingStrategy;


public class HibernateSettings {

	private Supplier<String> ddlAuto;

	private ImplicitNamingStrategy implicitNamingStrategy;

	private PhysicalNamingStrategy physicalNamingStrategy;

	private Collection<HibernatePropertiesCustomizer> hibernatePropertiesCustomizers;

	public HibernateSettings ddlAuto(Supplier<String> ddlAuto) {
		this.ddlAuto = ddlAuto;
		return this;
	}

	
	@Deprecated
	public HibernateSettings ddlAuto(String ddlAuto) {
		return ddlAuto(() -> ddlAuto);
	}

	public String getDdlAuto() {
		return (this.ddlAuto != null ? this.ddlAuto.get() : null);
	}

	public HibernateSettings implicitNamingStrategy(
			ImplicitNamingStrategy implicitNamingStrategy) {
		this.implicitNamingStrategy = implicitNamingStrategy;
		return this;
	}

	public ImplicitNamingStrategy getImplicitNamingStrategy() {
		return this.implicitNamingStrategy;
	}

	public HibernateSettings physicalNamingStrategy(
			PhysicalNamingStrategy physicalNamingStrategy) {
		this.physicalNamingStrategy = physicalNamingStrategy;
		return this;
	}

	public PhysicalNamingStrategy getPhysicalNamingStrategy() {
		return this.physicalNamingStrategy;
	}

	public HibernateSettings hibernatePropertiesCustomizers(
			Collection<HibernatePropertiesCustomizer> hibernatePropertiesCustomizers) {
		this.hibernatePropertiesCustomizers = new ArrayList<>(
				hibernatePropertiesCustomizers);
		return this;
	}

	public Collection<HibernatePropertiesCustomizer> getHibernatePropertiesCustomizers() {
		return this.hibernatePropertiesCustomizers;
	}

}
