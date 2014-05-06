#define NO_MAIN
#include <../../main.cpp>
#include <boost/test/minimal.hpp>

int test_main( int, char *[] )
{
    // simple test of problem without test throws and error cases
    y_clock::Time t;
    y_clock::Parameters p;

    t.fromString("03:00");
    p.fromString("deg", "mech");

    y_clock::AngleCalculator calculator(t, p);
    calculator.calculate();
    BOOST_CHECK(calculator.angleToString() == "90");

    t.fromString("15:00");
    p.fromString("rad", "mech");
    calculator.setTime(t);
    calculator.setParameters(p);
    calculator.calculate();
    BOOST_CHECK(calculator.angleToString() == "1.5708");

    t.fromString("09:00 PM");
    p.fromString("dms", "quar");
    calculator.setTime(t);
    calculator.setParameters(p);
    calculator.calculate();
    BOOST_CHECK(calculator.angleToString() == "90.0'0''");

    t.fromString("9:17 AM");
    p.fromString("rad", "quar");
    calculator.setTime(t);
    calculator.setParameters(p);
    calculator.calculate();
    BOOST_CHECK(calculator.angleToString() == "2.9322");

    t.fromString("19:48");
    p.fromString("dms", "mech");
    calculator.setTime(t);
    calculator.setParameters(p);
    calculator.calculate();
    BOOST_CHECK(calculator.angleToString() == "102.0'0''");

    return 0;
}
