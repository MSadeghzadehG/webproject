

package org.springframework.boot.configurationprocessor;

import java.io.File;
import java.io.IOException;
import java.util.Collections;

import javax.annotation.processing.ProcessingEnvironment;

import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TemporaryFolder;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.mock;


public class MetadataStoreTests {

	@Rule
	public final TemporaryFolder temp = new TemporaryFolder();

	private final ProcessingEnvironment environment = mock(ProcessingEnvironment.class);

	private final MetadataStore metadataStore = new MetadataStore(this.environment);

	@Test
	public void additionalMetadataIsLocatedInMavenBuild() throws IOException {
		File app = this.temp.newFolder("app");
		File classesLocation = new File(app, "target/classes");
		File metaInf = new File(classesLocation, "META-INF");
		metaInf.mkdirs();
		File additionalMetadata = new File(metaInf,
				"additional-spring-configuration-metadata.json");
		additionalMetadata.createNewFile();
		assertThat(
				this.metadataStore.locateAdditionalMetadataFile(new File(classesLocation,
						"META-INF/additional-spring-configuration-metadata.json")))
								.isEqualTo(additionalMetadata);
	}

	@Test
	public void additionalMetadataIsLocatedInGradle3Build() throws IOException {
		File app = this.temp.newFolder("app");
		File classesLocation = new File(app, "build/classes/main");
		File resourcesLocation = new File(app, "build/resources/main");
		File metaInf = new File(resourcesLocation, "META-INF");
		metaInf.mkdirs();
		File additionalMetadata = new File(metaInf,
				"additional-spring-configuration-metadata.json");
		additionalMetadata.createNewFile();
		assertThat(
				this.metadataStore.locateAdditionalMetadataFile(new File(classesLocation,
						"META-INF/additional-spring-configuration-metadata.json")))
								.isEqualTo(additionalMetadata);
	}

	@Test
	public void additionalMetadataIsLocatedInGradle4Build() throws IOException {
		File app = this.temp.newFolder("app");
		File classesLocation = new File(app, "build/classes/java/main");
		File resourcesLocation = new File(app, "build/resources/main");
		File metaInf = new File(resourcesLocation, "META-INF");
		metaInf.mkdirs();
		File additionalMetadata = new File(metaInf,
				"additional-spring-configuration-metadata.json");
		additionalMetadata.createNewFile();
		assertThat(
				this.metadataStore.locateAdditionalMetadataFile(new File(classesLocation,
						"META-INF/additional-spring-configuration-metadata.json")))
								.isEqualTo(additionalMetadata);
	}

	@Test
	public void additionalMetadataIsLocatedUsingLocationsOption() throws IOException {
		File app = this.temp.newFolder("app");
		File location = new File(app, "src/main/resources");
		File metaInf = new File(location, "META-INF");
		metaInf.mkdirs();
		File additionalMetadata = new File(metaInf,
				"additional-spring-configuration-metadata.json");
		additionalMetadata.createNewFile();
		given(this.environment.getOptions()).willReturn(Collections.singletonMap(
				ConfigurationMetadataAnnotationProcessor.ADDITIONAL_METADATA_LOCATIONS_OPTION,
				location.getAbsolutePath()));
		assertThat(this.metadataStore.locateAdditionalMetadataFile(new File(app, "foo")))
				.isEqualTo(additionalMetadata);
	}

}
