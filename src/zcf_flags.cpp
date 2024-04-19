#include "zcf/zcf_flags.hpp"
#include <sstream>
#include <fstream>
#include <string.h>

namespace zcf{

/// Option implementation /////////////////////////////////

Option::Option(const std::string& short_name, const std::string& long_name, std::string description)
    : short_name_(short_name), long_name_(long_name), description_(std::move(description)), attribute_(Attribute::optional)
{
    if (short_name.size() > 1)
        throw std::invalid_argument("length of short name must be <= 1: '" + short_name + "'");

    if (short_name.empty() && long_name.empty())
        throw std::invalid_argument("short and long name are empty");
}


char Option::short_name() const
{
    if (!short_name_.empty())
        return short_name_[0];
    return 0;
}


std::string Option::long_name() const
{
    return long_name_;
}


std::string Option::name(OptionName what_name, bool with_hypen) const
{
    if (what_name == OptionName::short_name)
        return short_name_.empty() ? "" : ((with_hypen ? "-" : "") + short_name_);
    if (what_name == OptionName::long_name)
        return long_name_.empty() ? "" : ((with_hypen ? "--" : "") + long_name_);
    return "";
}


std::string Option::description() const
{
    return description_;
}


void Option::set_attribute(const Attribute& attribute)
{
    attribute_ = attribute;
}


Attribute Option::attribute() const
{
    return attribute_;
}

/// Option implementation /////////////////////////////////

/// OptionParser implementation /////////////////////////////////

OptionParser::OptionParser(std::string description) : description_(std::move(description)){
}

Option_ptr OptionParser::find_option(const std::string& long_name) const{
    for (const auto& option : options_)
        if (option->long_name() == long_name)
            return option;
    return nullptr;
}

Option_ptr OptionParser::find_option(char short_name) const{
    for (const auto& option : options_)
        if (option->short_name() == short_name)
            return option;
    return nullptr;
}

inline void OptionParser::parse(const std::string& ini_filename)
{
    std::ifstream file(ini_filename.c_str());
    std::string line;

    auto trim = [](std::string& s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) { return !std::isspace(ch); }));
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) { return !std::isspace(ch); }).base(), s.end());
        return s;
    };

    auto trim_copy = [trim](const std::string& s) {
        std::string copy(s);
        return trim(copy);
    };

    auto split = [trim_copy](const std::string& s) -> std::pair<std::string, std::string> {
        size_t pos = s.find('=');
        if (pos == std::string::npos)
            return {"", ""};
        return {trim_copy(s.substr(0, pos)), trim_copy(s.substr(pos + 1, std::string::npos))};
    };

    std::string section;
    while (std::getline(file, line))
    {
        trim(line);
        if (line.empty())
            continue;
        if (line.front() == '#')
            continue;

        if ((line.front() == '[') && (line.back() == ']'))
        {
            section = trim_copy(line.substr(1, line.size() - 2));
            continue;
        }
        auto key_value = split(line);
        if (key_value.first.empty())
            continue;

        std::string key = section.empty() ? key_value.first : section + "." + key_value.first;
        Option_ptr option = find_option(key);
        if (option && (option->attribute() == Attribute::inactive))
            option = nullptr;

        if (option)
            option->parse(OptionName::long_name, key_value.second.c_str());
        else
            unknown_options_.push_back(key);
    }
}

void OptionParser::parse(int argc, const char* const argv[])
{
    for (int n = 1; n < argc; ++n)
    {
        const std::string arg(argv[n]);
        if (arg == "--")
        {
            /// from here on only non opt args
            for (int m = n + 1; m < argc; ++m)
                non_option_args_.emplace_back(argv[m]);
        }
        else if (arg.find("--") == 0)
        {
            /// long option arg
            std::string opt = arg.substr(2);
            std::string optarg;
            size_t equalIdx = opt.find('=');
            if (equalIdx != std::string::npos)
            {
                optarg = opt.substr(equalIdx + 1);
                opt.resize(equalIdx);
            }

            Option_ptr option = find_option(opt);
            if (option && (option->attribute() == Attribute::inactive))
                option = nullptr;
            if (option)
            {
                if (option->argument_type() == Argument::no)
                {
                    if (!optarg.empty())
                        option = nullptr;
                }
                else if (option->argument_type() == Argument::required)
                {
                    if (optarg.empty() && n < argc - 1)
                        optarg = argv[++n];
                }
            }

            if (option)
                option->parse(OptionName::long_name, optarg.c_str());
            else
                unknown_options_.push_back(arg);
        }
        else if (arg.find('-') == 0)
        {
            /// short option arg
            std::string opt = arg.substr(1);
            bool unknown = false;
            for (size_t m = 0; m < opt.size(); ++m)
            {
                char c = opt[m];
                std::string optarg;

                Option_ptr option = find_option(c);
                if (option && (option->attribute() == Attribute::inactive))
                    option = nullptr;
                if (option)
                {
                    if (option->argument_type() == Argument::required)
                    {
                        /// use the rest of the current argument as optarg
                        optarg = opt.substr(m + 1);
                        /// or the next arg
                        if (optarg.empty() && n < argc - 1)
                            optarg = argv[++n];
                        m = opt.size();
                    }
                    else if (option->argument_type() == Argument::optional)
                    {
                        /// use the rest of the current argument as optarg
                        optarg = opt.substr(m + 1);
                        m = opt.size();
                    }
                }

                if (option)
                    option->parse(OptionName::short_name, optarg.c_str());
                else
                    unknown = true;
            }
            if (unknown)
                unknown_options_.push_back(arg);
        }
        else
        {
            non_option_args_.push_back(arg);
        }
    }

    for (auto& opt : options_)
    {
        if ((opt->attribute() == Attribute::required) && !opt->is_set())
        {
            std::string option = opt->long_name().empty() ? std::string(1, opt->short_name()) : opt->long_name();
            throw invalid_option(opt.get(), invalid_option::Error::missing_option, "option \"" + option + "\" is required");
        }
    }
}


void OptionParser::reset()
{
    unknown_options_.clear();
    non_option_args_.clear();
    for (auto& opt : options_)
        opt->clear();
}


std::string OptionParser::help(const Attribute& max_attribute) const
{
    ConsoleOptionPrinter option_printer(this);
    return option_printer.print(max_attribute);
}

/// OptionParser implementation /////////////////////////////////

/// Switch implementation /////////////////////////////////
Switch::Switch(const std::string& short_name, const std::string& long_name, const std::string& description, bool* assign_to)
    : Value<bool>(short_name, long_name, description, false, assign_to){
}

void Switch::parse(OptionName /*what_name*/, const char* /*value*/){
    add_value(true);
}

Argument Switch::argument_type() const {
    return Argument::no;
}
/// ConsoleOptionPrinter implementation /////////////////////////////////

ConsoleOptionPrinter::ConsoleOptionPrinter(const OptionParser* option_parser) : OptionPrinter(option_parser)
{
}

std::string ConsoleOptionPrinter::to_string(Option_ptr option) const
{
    std::stringstream line;
    if (option->short_name() != 0)
    {
        line << "  -" << option->short_name();
        if (!option->long_name().empty())
            line << ", ";
    }
    else
        line << "  ";
    if (!option->long_name().empty())
        line << "--" << option->long_name();

    if (option->argument_type() == Argument::required)
    {
        line << " arg";
        std::stringstream defaultStr;
        if (option->get_default(defaultStr))
        {
            if (!defaultStr.str().empty())
                line << " (=" << defaultStr.str() << ")";
        }
    }
    else if (option->argument_type() == Argument::optional)
    {
        std::stringstream defaultStr;
        if (option->get_default(defaultStr))
            line << " [=arg(=" << defaultStr.str() << ")]";
    }

    return line.str();
}

std::string ConsoleOptionPrinter::print(const Attribute& max_attribute) const
{
    if (option_parser_ == nullptr)
        return "";

    if (max_attribute < Attribute::optional)
        throw std::invalid_argument("attribute must be 'optional', 'advanced', or 'default'");

    std::stringstream s;
    if (!option_parser_->description().empty())
        s << option_parser_->description() << ":\n";

    size_t optionRightMargin(20);
    const size_t maxDescriptionLeftMargin(40);
    //	const size_t descriptionRightMargin(80);

    for (const auto& option : option_parser_->options())
        optionRightMargin = std::max(optionRightMargin, to_string(option).size() + 2);
    optionRightMargin = std::min(maxDescriptionLeftMargin - 2, optionRightMargin);

    for (const auto& option : option_parser_->options())
    {
        if ((option->attribute() <= Attribute::hidden) || (option->attribute() > max_attribute))
            continue;
        std::string optionStr = to_string(option);
        if (optionStr.size() < optionRightMargin)
            optionStr.resize(optionRightMargin, ' ');
        else
            optionStr += "\n" + std::string(optionRightMargin, ' ');
        s << optionStr;

        std::string line;
        std::vector<std::string> lines;
        std::stringstream description(option->description());
        while (std::getline(description, line, '\n'))
            lines.push_back(line);

        std::string empty(optionRightMargin, ' ');
        for (size_t n = 0; n < lines.size(); ++n)
        {
            if (n > 0)
                s << "\n" << empty;
            s << lines[n];
        }
        s << "\n";
    }

    return s.str();
}
/// ConsoleOptionPrinter implementation /////////////////////////////////
}//!namespace zcf