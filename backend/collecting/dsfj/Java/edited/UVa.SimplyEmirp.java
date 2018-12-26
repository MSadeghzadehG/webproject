



import java.math.BigInteger;
import java.util.Scanner;

public class SimplyEmirp {

	public static void main(String[] args) {
		Scanner input = new Scanner(System.in);
		while (input.hasNext()) {
			String inputGiven = input.next();
			BigInteger number = new BigInteger(inputGiven);
			if (!number.isProbablePrime(10)) {
				System.out.println(number + " is not prime.");
			} else {
				String numberReversedAsString = new StringBuilder(
						number.toString()).reverse().toString();
				BigInteger numberReversed = new BigInteger(
						numberReversedAsString);
				if (numberReversed.isProbablePrime(10)
						&& numberReversed.compareTo(number) != 0) {
					System.out.println(number + " is emirp.");
				} else {
					System.out.println(number + " is prime.");
				}
			}
		}
	}
}
