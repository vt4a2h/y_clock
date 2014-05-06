#include <iostream>
#include <iomanip>
#include <unordered_map>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

namespace y_clock {

    struct Time {
        bool isValid() const;
        void fromString(const std::string &s);

        unsigned short h_;
        unsigned short m_;
        std::string    format_;
    };

    struct Parameters {
        enum AngleFormat : int {deg, rad, dms};
        enum ClockType   : int {quar, mech};

        typedef std::unordered_map<std::string, AngleFormat> AngleMap;
        typedef std::unordered_map<std::string, ClockType>   ClockMap;

        void fromString(const std::string &angleFormat, const std::string &clockType);

        AngleFormat angleFormat_;
        ClockType   clockType_;
        const static AngleMap kAngleFormatStr_;
        const static ClockMap kClockTypeStr_;
    };

    class AngleCalculator
    {
    public:
        AngleCalculator(const Time &time, const Parameters &parameters);

        void calculate();
        double angle() const;
        std::string angleToString() const;

        Time time() const;
        void setTime(const Time &time);

        Parameters parameters() const;
        void setParameters(const Parameters &parameters);

    private:
        Time time_;
        Parameters parameters_;
        double angle_;
    };

}

#if !(defined NO_MAIN)

int main(int argc, char **argv)
{
    (void)argc;
    if (argv[1] && argv[2] && argv[3]) {
        try {
            y_clock::Time t;
            t.fromString(argv[1]);

            if (t.isValid()) {
                y_clock::Parameters p;
                p.fromString(argv[2], argv[3]);

                y_clock::AngleCalculator calclulator(t, p);
                calclulator.calculate();
                std::cout << calclulator.angleToString() << std::endl;
            } else {
                throw std::logic_error("time is not valid");
            }
        } catch (std::logic_error &e) {
            std::cout << "Logic error: " << e.what() << std::endl;
        } catch (std::exception &e) {
            std::cout << "Unknown error: " << e.what() << std::endl;
        }

        return 0;
    }

    std::cout << "The program requires 3 arguments:\n"
                 "[time (hh:mm, hh:mm AM/PM)]\n"
                 "[output format (deg, rad, dms)]\n"
                 "[clock type (quar, mech)]\n\n"
                 "Example: \"10:33 PM\" deg quar" << std::endl;
    return -1;
}

#endif

namespace y_clock {

    void Time::fromString(const std::string &s)
    {
        using boost::spirit::qi::_1;
        using boost::spirit::qi::uint_parser;
        using boost::spirit::qi::char_;
        using boost::spirit::qi::string;
        using boost::spirit::qi::space;
        using boost::phoenix::ref;

        unsigned short h, m;
        uint_parser<unsigned short, 10, 1, 2> u_;
        boost::spirit::qi::rule<const char *, void()> time_parser =
                u_[ref(h) = _1] >> char_(':') >> u_[ref(m) = _1];

        std::string f;
        auto to_str = [&f](std::string str){ f = str; };
        boost::spirit::qi::rule<const char *, void()> format_parser =
                -(space >> (string("AM") | string("PM"))[to_str]);

        const char *first = s.data();
        const char *end   = first + s.size();
        bool success = boost::spirit::qi::parse(first, end, time_parser >> format_parser);

        if (!success || first != end) throw std::logic_error("time format is not valid");

        h_ = h; m_ = m; format_ = f;
    }

    bool Time::isValid() const
    {
        return ((format_.empty() && h_ < 24) || (!format_.empty() && h_ < 12))
                && m_ < 60;
    }

    const Parameters::AngleMap Parameters::kAngleFormatStr_ = {{"deg", deg},   {"rad", rad}, {"dms", dms}};
    const Parameters::ClockMap Parameters::kClockTypeStr_   = {{"quar", quar}, {"mech", mech}};

    void Parameters::fromString(const std::string &angleFormat, const std::string &clockType)
    {
        AngleFormat format;
        ClockType   type;

        auto formatIt = kAngleFormatStr_.find(angleFormat);
        if (formatIt != kAngleFormatStr_.end())
            format = formatIt->second;
        else
            throw std::logic_error("angle format is not valid");

        auto typeIt = kClockTypeStr_.find(clockType);
        if (typeIt != kClockTypeStr_.end())
            type = typeIt->second;
        else
            throw std::logic_error("clock format is not valid");

        angleFormat_ = format;
        clockType_   = type;
    }

    AngleCalculator::AngleCalculator(const Time &time, const Parameters &parameters)
        : time_(time)
        , parameters_(parameters)
        , angle_(0.0)
    {}

    void AngleCalculator::calculate()
    {
        if (!time_.isValid()) throw std::logic_error("Time is not valid");

        // set hours to interval [0..11]
        unsigned short h(time_.h_ >= 12 ? time_.h_ - 12 : time_.h_);

        /*
         * (h * 5) -> hours to "minutes" representation
         * (- m_)  -> minus current minutes
         * fabs()  -> fix less than zero
         * (* 6.0) -> to deg
         */
        angle_ = fabs(h * 5.0 - time_.m_) * 6.0;

        // (+ m / 2.0) or (+ 6 * m / 12) -> hour hand moves 12 times slower then minute hand
        if (parameters_.clockType_ == Parameters::mech) angle_ += time_.m_ / 2.0;

        if (angle_ > 180) angle_ = 360 - angle_;
    }

    double AngleCalculator::angle() const
    {
        return angle_;
    }

    std::string AngleCalculator::angleToString() const
    {
        using namespace boost;
        switch (parameters_.angleFormat_) {
            case Parameters::deg:
                return lexical_cast<std::string>(angle_);
            case Parameters::rad:
            {
                double r = angle_ * math::constants::pi<double>() / 180.0;
                return (format("%d") % io::group(std::setprecision(5), r)).str();
            }
            case Parameters::dms:
            {
                double d = floor(angle_),
                       rem = (angle_ - d) * 60,
                       m = floor(rem),
                       s = (rem - m) * 60;
                return (format("%d.%d'%d''") % d % m % s).str();

            }
            default:
                return "Unknown result type";
        }
    }

    Time AngleCalculator::time() const
    {
        return time_;
    }

    void AngleCalculator::setTime(const Time &time)
    {
        time_ = time;
    }

    Parameters AngleCalculator::parameters() const
    {
        return parameters_;
    }

    void AngleCalculator::setParameters(const Parameters &parameters)
    {
        parameters_ = parameters;
    }
}
