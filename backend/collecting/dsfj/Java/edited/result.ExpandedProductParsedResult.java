



package com.google.zxing.client.result;

import java.util.Map;


public final class ExpandedProductParsedResult extends ParsedResult {

  public static final String KILOGRAM = "KG";
  public static final String POUND = "LB";

  private final String rawText;
  private final String productID;
  private final String sscc;
  private final String lotNumber;
  private final String productionDate;
  private final String packagingDate;
  private final String bestBeforeDate;
  private final String expirationDate;
  private final String weight;
  private final String weightType;
  private final String weightIncrement;
  private final String price;
  private final String priceIncrement;
  private final String priceCurrency;
    private final Map<String,String> uncommonAIs;

  public ExpandedProductParsedResult(String rawText,
                                     String productID,
                                     String sscc,
                                     String lotNumber,
                                     String productionDate,
                                     String packagingDate,
                                     String bestBeforeDate,
                                     String expirationDate,
                                     String weight,
                                     String weightType,
                                     String weightIncrement,
                                     String price,
                                     String priceIncrement,
                                     String priceCurrency,
                                     Map<String,String> uncommonAIs) {
    super(ParsedResultType.PRODUCT);
    this.rawText = rawText;
    this.productID = productID;
    this.sscc = sscc;
    this.lotNumber = lotNumber;
    this.productionDate = productionDate;
    this.packagingDate = packagingDate;
    this.bestBeforeDate = bestBeforeDate;
    this.expirationDate = expirationDate;
    this.weight = weight;
    this.weightType = weightType;
    this.weightIncrement = weightIncrement;
    this.price = price;
    this.priceIncrement = priceIncrement;
    this.priceCurrency = priceCurrency;
    this.uncommonAIs = uncommonAIs;
  }

  @Override
  public boolean equals(Object o) {
    if (!(o instanceof ExpandedProductParsedResult)) {
      return false;
    }

    ExpandedProductParsedResult other = (ExpandedProductParsedResult) o;

    return equalsOrNull(productID, other.productID)
        && equalsOrNull(sscc, other.sscc)
        && equalsOrNull(lotNumber, other.lotNumber)
        && equalsOrNull(productionDate, other.productionDate)
        && equalsOrNull(bestBeforeDate, other.bestBeforeDate)
        && equalsOrNull(expirationDate, other.expirationDate)
        && equalsOrNull(weight, other.weight)
        && equalsOrNull(weightType, other.weightType)
        && equalsOrNull(weightIncrement, other.weightIncrement)
        && equalsOrNull(price, other.price)
        && equalsOrNull(priceIncrement, other.priceIncrement)
        && equalsOrNull(priceCurrency, other.priceCurrency)
        && equalsOrNull(uncommonAIs, other.uncommonAIs);
  }

  private static boolean equalsOrNull(Object o1, Object o2) {
    return o1 == null ? o2 == null : o1.equals(o2);
  }

  @Override
  public int hashCode() {
    int hash = 0;
    hash ^= hashNotNull(productID);
    hash ^= hashNotNull(sscc);
    hash ^= hashNotNull(lotNumber);
    hash ^= hashNotNull(productionDate);
    hash ^= hashNotNull(bestBeforeDate);
    hash ^= hashNotNull(expirationDate);
    hash ^= hashNotNull(weight);
    hash ^= hashNotNull(weightType);
    hash ^= hashNotNull(weightIncrement);
    hash ^= hashNotNull(price);
    hash ^= hashNotNull(priceIncrement);
    hash ^= hashNotNull(priceCurrency);
    hash ^= hashNotNull(uncommonAIs);
    return hash;
  }

  private static int hashNotNull(Object o) {
    return o == null ? 0 : o.hashCode();
  }

  public String getRawText() {
    return rawText;
  }

  public String getProductID() {
    return productID;
  }

  public String getSscc() {
    return sscc;
  }

  public String getLotNumber() {
    return lotNumber;
  }

  public String getProductionDate() {
    return productionDate;
  }

  public String getPackagingDate() {
    return packagingDate;
  }

  public String getBestBeforeDate() {
    return bestBeforeDate;
  }

  public String getExpirationDate() {
    return expirationDate;
  }

  public String getWeight() {
    return weight;
  }

  public String getWeightType() {
    return weightType;
  }

  public String getWeightIncrement() {
    return weightIncrement;
  }

  public String getPrice() {
    return price;
  }

  public String getPriceIncrement() {
    return priceIncrement;
  }

  public String getPriceCurrency() {
    return priceCurrency;
  }

  public Map<String,String> getUncommonAIs() {
    return uncommonAIs;
  }

  @Override
  public String getDisplayResult() {
    return String.valueOf(rawText);
  }
}
