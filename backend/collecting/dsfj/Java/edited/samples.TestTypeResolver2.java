package jadx.samples;


public class TestTypeResolver2 extends AbstractTest {

	private static String result = "";

	public void testOverloadedMethods() {
		Object s1 = "The";
		Object s2 = "answer";
		doPrint((Object) "You should know:");
		for (int i = 0; i < 2; i++) {
			doPrint(s1);
			doPrint(s2);
			s1 = "is";
			s2 = new Integer(42);
		}
	}

	private static void doPrint(String s1) {
		fail();
	}

	private static void doPrint(Integer s1) {
		fail();
	}

	private static void doPrint(Object s1) {
				result += s1 + " ";
	}

	@Override
	public boolean testRun() throws Exception {
		testOverloadedMethods();

		assertEquals(result, "You should know: The answer is 42 ");
		return true;
	}

	public static void main(String[] args) throws Exception {
		(new TestTypeResolver2()).testRun();
	}
}
