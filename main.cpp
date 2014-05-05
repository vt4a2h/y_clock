#include <iostream>
#include <unordered_map>
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

    class AngleClaculator;
}

int main(int argc, char **argv)
{
    if (argv[1] && argv[2] && argv[3]) {
        try {
            y_clock::Time t;
            t.fromString(argv[1]);

            if (t.isValid()) {
                y_clock::Parameters p;
                p.fromString(argv[2], argv[3]);
            } else {
                std::cout << "Time is not valid" << std::endl;
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
}
