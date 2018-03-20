

package org.springframework.boot.autoconfigure.jackson;

import java.util.EnumMap;
import java.util.Locale;
import java.util.Map;
import java.util.TimeZone;

import com.fasterxml.jackson.annotation.JsonInclude;
import com.fasterxml.jackson.core.JsonGenerator;
import com.fasterxml.jackson.core.JsonParser;
import com.fasterxml.jackson.databind.DeserializationFeature;
import com.fasterxml.jackson.databind.MapperFeature;
import com.fasterxml.jackson.databind.SerializationFeature;

import org.springframework.boot.context.properties.ConfigurationProperties;


@ConfigurationProperties(prefix = "spring.jackson")
public class JacksonProperties {

	
	private String dateFormat;

	
	private String jodaDateTimeFormat;

	
	private String propertyNamingStrategy;

	
	private Map<SerializationFeature, Boolean> serialization = new EnumMap<>(
			SerializationFeature.class);

	
	private Map<DeserializationFeature, Boolean> deserialization = new EnumMap<>(
			DeserializationFeature.class);

	
	private Map<MapperFeature, Boolean> mapper = new EnumMap<>(MapperFeature.class);

	
	private Map<JsonParser.Feature, Boolean> parser = new EnumMap<>(
			JsonParser.Feature.class);

	
	private Map<JsonGenerator.Feature, Boolean> generator = new EnumMap<>(
			JsonGenerator.Feature.class);

	
	private JsonInclude.Include defaultPropertyInclusion;

	
	private TimeZone timeZone = null;

	
	private Locale locale;

	public String getDateFormat() {
		return this.dateFormat;
	}

	public void setDateFormat(String dateFormat) {
		this.dateFormat = dateFormat;
	}

	public String getJodaDateTimeFormat() {
		return this.jodaDateTimeFormat;
	}

	public void setJodaDateTimeFormat(String jodaDataTimeFormat) {
		this.jodaDateTimeFormat = jodaDataTimeFormat;
	}

	public String getPropertyNamingStrategy() {
		return this.propertyNamingStrategy;
	}

	public void setPropertyNamingStrategy(String propertyNamingStrategy) {
		this.propertyNamingStrategy = propertyNamingStrategy;
	}

	public Map<SerializationFeature, Boolean> getSerialization() {
		return this.serialization;
	}

	public Map<DeserializationFeature, Boolean> getDeserialization() {
		return this.deserialization;
	}

	public Map<MapperFeature, Boolean> getMapper() {
		return this.mapper;
	}

	public Map<JsonParser.Feature, Boolean> getParser() {
		return this.parser;
	}

	public Map<JsonGenerator.Feature, Boolean> getGenerator() {
		return this.generator;
	}

	public JsonInclude.Include getDefaultPropertyInclusion() {
		return this.defaultPropertyInclusion;
	}

	public void setDefaultPropertyInclusion(
			JsonInclude.Include defaultPropertyInclusion) {
		this.defaultPropertyInclusion = defaultPropertyInclusion;
	}

	public TimeZone getTimeZone() {
		return this.timeZone;
	}

	public void setTimeZone(TimeZone timeZone) {
		this.timeZone = timeZone;
	}

	public Locale getLocale() {
		return this.locale;
	}

	public void setLocale(Locale locale) {
		this.locale = locale;
	}

}
