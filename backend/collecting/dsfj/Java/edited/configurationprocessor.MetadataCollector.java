

package org.springframework.boot.configurationprocessor;

import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import javax.annotation.processing.ProcessingEnvironment;
import javax.annotation.processing.RoundEnvironment;
import javax.lang.model.element.Element;
import javax.lang.model.element.TypeElement;

import org.springframework.boot.configurationprocessor.metadata.ConfigurationMetadata;
import org.springframework.boot.configurationprocessor.metadata.ItemMetadata;


public class MetadataCollector {

	private final Set<ItemMetadata> metadataItems = new LinkedHashSet<>();

	private final ProcessingEnvironment processingEnvironment;

	private final ConfigurationMetadata previousMetadata;

	private final TypeUtils typeUtils;

	private final Set<String> processedSourceTypes = new HashSet<>();

	
	public MetadataCollector(ProcessingEnvironment processingEnvironment,
			ConfigurationMetadata previousMetadata) {
		this.processingEnvironment = processingEnvironment;
		this.previousMetadata = previousMetadata;
		this.typeUtils = new TypeUtils(processingEnvironment);
	}

	public void processing(RoundEnvironment roundEnv) {
		for (Element element : roundEnv.getRootElements()) {
			markAsProcessed(element);
		}
	}

	private void markAsProcessed(Element element) {
		if (element instanceof TypeElement) {
			this.processedSourceTypes.add(this.typeUtils.getQualifiedName(element));
		}
	}

	public void add(ItemMetadata metadata) {
		this.metadataItems.add(metadata);
	}

	public boolean hasSimilarGroup(ItemMetadata metadata) {
		if (!metadata.isOfItemType(ItemMetadata.ItemType.GROUP)) {
			throw new IllegalStateException("item " + metadata + " must be a group");
		}
		for (ItemMetadata existing : this.metadataItems) {
			if (existing.isOfItemType(ItemMetadata.ItemType.GROUP)
					&& existing.getName().equals(metadata.getName())
					&& existing.getType().equals(metadata.getType())) {
				return true;
			}
		}
		return false;
	}

	public ConfigurationMetadata getMetadata() {
		ConfigurationMetadata metadata = new ConfigurationMetadata();
		for (ItemMetadata item : this.metadataItems) {
			metadata.add(item);
		}
		if (this.previousMetadata != null) {
			List<ItemMetadata> items = this.previousMetadata.getItems();
			for (ItemMetadata item : items) {
				if (shouldBeMerged(item)) {
					metadata.add(item);
				}
			}
		}
		return metadata;
	}

	private boolean shouldBeMerged(ItemMetadata itemMetadata) {
		String sourceType = itemMetadata.getSourceType();
		return (sourceType != null && !deletedInCurrentBuild(sourceType)
				&& !processedInCurrentBuild(sourceType));
	}

	private boolean deletedInCurrentBuild(String sourceType) {
		return this.processingEnvironment.getElementUtils()
				.getTypeElement(sourceType) == null;
	}

	private boolean processedInCurrentBuild(String sourceType) {
		return this.processedSourceTypes.contains(sourceType);
	}

}
