


import java.math.BigInteger;
import java.util.Scanner;

public class SimplifyingFractions {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		int numberOfTestCases = input.nextInt();
		while (numberOfTestCases != 0) {
			String pAsString = input.next();
			input.next();
			String qAsString = input.next();
			BigInteger p = new BigInteger(pAsString);
			BigInteger q = new BigInteger(qAsString);
			BigInteger greatestCommonDivisor = p.gcd(q);
			if (!greatestCommonDivisor.equals(BigInteger.ONE)) {
				p = p.divide(greatestCommonDivisor);
				q = q.divide(greatestCommonDivisor);
			}
			System.out.println(p + " / " + q);
			numberOfTestCases--;
		}
	}
}
