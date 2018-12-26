


import java.math.BigInteger;
import java.util.Scanner;

public class MultipleOfSeventeen {
	private static final BigInteger BIGINTEGER_FIVE = new BigInteger("5");
	private static final BigInteger BIGINTEGER_SEVENTEEN = new BigInteger("17");
	private static final BigInteger BIGINTEGER_ZERO = new BigInteger("0");

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		while (input.hasNext()) {
			BigInteger number = input.nextBigInteger();
			if (number.equals(BIGINTEGER_ZERO)) {
				break;
			}
			BigInteger lastDigit = number.mod(BigInteger.TEN);
			number = number.divide(BigInteger.TEN);
			BigInteger product5D = lastDigit.multiply(BIGINTEGER_FIVE);
			BigInteger difference = number.subtract(product5D);
			if (difference.mod(BIGINTEGER_SEVENTEEN).equals(BIGINTEGER_ZERO)) {
				System.out.println("1");
			} else {
				System.out.println("0");
			}
		}
	}
}
