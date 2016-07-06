#include <mitsuba/core/argparser.h>
#include <mitsuba/core/logger.h>
#include <mitsuba/core/string.h>

NAMESPACE_BEGIN(mitsuba)

void ArgParser::Arg::append(const std::string &value) {
    if (m_present) {
        if (!m_next)
            m_next = new Arg(*this);
        m_next->append(value);
    } else {
        m_present = true;
        m_value = value;
    }
}

size_t ArgParser::Arg::count() const {
    const Arg *value = this;
    size_t nargs = 0;
    while (value) {
        nargs += value->m_present ? 1 : 0;
        value = value->next();
    }
    return nargs;
}

int ArgParser::Arg::asInt() const {
    try {
        return std::stoi(m_value);
    } catch (const std::logic_error &) {
        Throw("Argument \"%s\": value \"%s\" is not an integer!", m_prefixes[0], m_value);
    }
}

Float ArgParser::Arg::asFloat() const {
    try {
        return (Float) std::stod(m_value);
    } catch (const std::logic_error &) {
        Throw("Argument \"%s\": value \"%s\" is not a floating point value!", m_prefixes[0], m_value);
    }
}

void ArgParser::parse(int argc, const char **argv) {
    std::vector<std::string> cmdline(argc);
    for (int i = 0; i < argc; ++i)
        cmdline[i] = argv[i];

    if (!cmdline.empty())
        m_executableName = cmdline[0];

    for (size_t i = 1; i < cmdline.size(); ++i) {
        bool found = false;

        for (Arg *arg: m_args) {
            for (const std::string &prefix : arg->m_prefixes) {
                const bool longForm = string::startsWith(prefix, "--");
                const bool shortForm = string::startsWith(prefix, "-") && !longForm;
                const bool other = prefix.empty() && arg->m_extra;
                const bool prefixFound = string::startsWith(cmdline[i], prefix);

                if (shortForm && prefixFound) {
                    std::string suffix = cmdline[i].substr(prefix.length());
                    if (!suffix.empty()) {
                        if (!arg->m_extra)
                            suffix = "-" + suffix;
                        cmdline.insert(cmdline.begin() + i + 1, suffix);
                    }
                    found = true;
                } else if (longForm && prefixFound) {
                    found = true;
                } else if (other) {
                    cmdline.insert(cmdline.begin() + i + 1, cmdline[i]);
                    found = true;
                }

                if (found) {
                    if (arg->m_extra) {
                        if (i + 1 >= cmdline.size() || string::startsWith(cmdline[i+1], "-"))
                            Throw("Missing/invalid argument for argument \"%s\"", prefix);
                        arg->append(cmdline[++i]);
                    } else {
                        arg->append();
                    }
                    break;
                }
            }
            if (found)
                break;
        }

        if (!found)
            Throw("Argument \"%s\" was not recognized!", cmdline[i]);
    }
}

NAMESPACE_END(mitsuba)
