#ifndef ZCF_FLAGS_HPP_
#define ZCF_FLAGS_HPP_

#include <zcf/zcf_string.hpp>

/**
 * class option used to define some options
 * 
 */
namespace zcf{

/// Option's argument type
/**
 * Switch has "no" argument
 * Value has "required" argument
 * Implicit has "optional" argument
 */
enum class Argument
{
    no = 0,   // option never takes an argument
    required, // option always requires an argument
    optional  // option may take an argument
};

/// Option's attribute
/**
 * inactive: Option is not set and will not be parsed
 * hidden:   Option is active, but will not show up in the help message
 * required: Option must be set on the command line. Otherwise an exception will be thrown
 * optional: Option must not be set. Default attribute.
 * advanced: Option is advanced and will only show up in the advanced help message
 * expoert:  Option is expert and will only show up in the expert help message
 */
enum class Attribute
{
    inactive = 0,
    hidden = 1,
    required = 2,
    optional = 3,
    advanced = 4,
    expert = 5
};

/// Option name type. Used in invalid_option exception.
/**
 * unspecified: not specified
 * short_name:  The option's short name
 * long_name:   The option's long name
 */
enum class OptionName
{
    unspecified,
    short_name,
    long_name
};

class Option;
class invalid_option : public std::invalid_argument
{
public:
    enum class Error
    {
        missing_argument,
        invalid_argument,
        too_many_arguments,
        missing_option
    };

    invalid_option(const Option* option, invalid_option::Error error, OptionName what_name, std::string value, const std::string& text)
        : std::invalid_argument(text.c_str()), option_(option), error_(error), what_name_(what_name), value_(std::move(value))
    {
    }

    invalid_option(const Option* option, invalid_option::Error error, const std::string& text)
        : invalid_option(option, error, OptionName::unspecified, "", text)
    {
    }

    const Option* option() const
    {
        return option_;
    }

    Error error() const
    {
        return error_;
    }

    OptionName what_name() const
    {
        return what_name_;
    }

    std::string value() const
    {
        return value_;
    }

private:
    const Option* option_;
    Error error_;
    OptionName what_name_;
    std::string value_;
};

/// Abstract Base class for Options
/**
 * Base class for Options
 * holds just configuration data, no runtime data.
 * Option is not bound to a special type "T"
 */
class Option
{
    friend class OptionParser;

public:
    /// Construct an Option
    /// @param short_name the options's short name. Must be empty or one character.
    /// @param long_name the option's long name. Can be empty.
    /// @param description the Option's description that will be shown in the help message
    Option(const std::string& short_name, const std::string& long_name, std::string description);

    /// Destructor
    virtual ~Option() = default;

    /// default copy constructor
    Option(const Option&) = default;

    /// default move constructor
    Option(Option&&) = default;

    /// default assignement operator
    Option& operator=(const Option&) = default;

    /// default move assignement operator
    Option& operator=(Option&&) = default;

    /// Get the Option's short name
    /// @return character of the options's short name or 0 if no short name is defined
    char short_name() const;

    /// Get the Option's long name
    /// @return the long name of the Option. Empty string if no long name is defined
    std::string long_name() const;

    /// Get the Option's long or short name
    /// @param what_name the option's name to return
    /// @param what_hyphen preced the returned name with (double-)hypen
    /// @return the requested name of the Option. Empty string if not defined.
    std::string name(OptionName what_name, bool with_hypen = false) const;

    /// Get the Option's description
    /// @return the description
    std::string description() const;

    /// Get the Option's default value
    /// @param out stream to write the default value to
    /// @return true if a default value is available, false if not
    virtual bool get_default(std::ostream& out) const = 0;

    /// Set the Option's attribute
    /// @param attribute
    void set_attribute(const Attribute& attribute);

    /// Get the Option's attribute
    /// @return the Options's attribute
    Attribute attribute() const;

    /// Get the Option's argument type
    /// @return argument type (no, required, optional)
    virtual Argument argument_type() const = 0;

    /// Check how often the Option is set on command line
    /// @return the Option's count on command line
    virtual size_t count() const = 0;

    /// Check if the Option is set
    /// @return true if set at least once
    virtual bool is_set() const = 0;


protected:
    /// Parse the command line option and fill the internal data structure
    /// @param what_name short or long option name
    /// @param value the value as given on command line
    virtual void parse(OptionName what_name, const char* value) = 0;

    /// Clear the internal data structure
    virtual void clear() = 0;

    std::string short_name_;
    std::string long_name_;
    std::string description_;
    Attribute attribute_;
};

using Option_ptr = std::shared_ptr<Option>;

/// Value option with optional default value
/**
 * Value option with optional default value
 * If set, it requires an argument
 */
template <class T>
class Value : public Option
{
public:
    /// Construct an Value Option
    /// @param short_name the option's short name. Must be empty or one character.
    /// @param long_name the option's long name. Can be empty.
    /// @param description the Option's description that will be shown in the help message
    Value(const std::string& short_name, const std::string& long_name, const std::string& description): Option(short_name, long_name, description), assign_to_(nullptr){}

    /// Construct an Value Option
    /// @param short_name the option's short name. Must be empty or one character.
    /// @param long_name the option's long name. Can be empty.
    /// @param description the Option's description that will be shown in the help message
    /// @param default_val the Option's default value
    /// @param assign_to pointer to a variable to assign the parsed command line value to
    Value(const std::string& short_name, const std::string& long_name, const std::string& description, const T& default_val, T* assign_to = nullptr): Value<T>(short_name, long_name, description){
        assign_to_ = assign_to;
        set_default(default_val);
    }

    size_t count() const override {
        return values_.size();
    }

    bool is_set() const override {
        return !values_.empty();
    }

    /// Assign the last parsed command line value to "var"
    /// @param var pointer to the variable where is value is written to
    void assign_to(T* var){
        assign_to_ = var;
        update_reference();
    }

    /// Manually set the Option's value. Deletes current value(s)
    /// @param value the new value of the option
    void set_value(const T& value){
        clear();
        add_value(value);
    }

    /// Get the Option's value. Will throw if option at index idx is not available
    /// @param idx the zero based index of the value (if set multiple times)
    /// @return the Option's value at index "idx"
    T value(size_t idx = 0) const{
        if (!this->is_set() && default_)
            return *default_;

        if (!is_set() || (idx >= count()))
        {
            std::stringstream optionStr;
            if (!is_set())
                optionStr << "option not set: \"";
            else
                optionStr << "index out of range (" << idx << ") for \"";

            if (short_name() != 0)
                optionStr << "-" << short_name();
            else
                optionStr << "--" << long_name();

            optionStr << "\"";
            throw std::out_of_range(optionStr.str());
        }

        return values_[idx];
    }

    /// Get the Option's value, return default_value if not set.
    /// @param default_value return value if value is not set
    /// @param idx the zero based index of the value (if set multiple times)
    /// @return the Option's value at index "idx" or the default value or default_value
    T value_or(const T& default_value, size_t idx = 0) const{
        if (idx < values_.size())
            return values_[idx];
        else if (default_)
            return *default_;
        else
            return default_value;
    }

    /// Set the Option's default value
    /// @param value the default value if not specified on command line
    void set_default(const T& value){
        this->default_.reset(new T);
        *this->default_ = value;
        update_reference();
    }

    /// Check if the Option has a default value
    /// @return true if the Option has a default value
    bool has_default() const{
        return (this->default_ != nullptr);
    }

    /// Get the Option's default value. Will throw if no default is set.
    /// @return the Option's default value
    T get_default() const{
        if (!has_default())
            throw std::runtime_error("no default value set");
        return *this->default_;
    }
    bool get_default(std::ostream& out) const override{
        if (!has_default())
            return false;
        out << *this->default_;
        return true;
    }

    Argument argument_type() const override {
        return Argument::required;
    }

protected:
    void parse(OptionName what_name, const char* value) override {
        T parsed_value;
        std::string strValue;
        if (value != nullptr)
            strValue = value;

        std::istringstream is(strValue);
        int valuesRead = 0;
        while (is.good())
        {
            if (is.peek() != EOF)
                is >> parsed_value;
            else
                break;

            valuesRead++;
        }

        if (is.fail())
            throw invalid_option(this, invalid_option::Error::invalid_argument, what_name, value,
                                "invalid argument for " + name(what_name, true) + ": '" + strValue + "'");

        if (valuesRead > 1)
            throw invalid_option(this, invalid_option::Error::too_many_arguments, what_name, value,
                                "too many arguments for " + name(what_name, true) + ": '" + strValue + "'");

        if (strValue.empty())
            throw invalid_option(this, invalid_option::Error::missing_argument, what_name, "", "missing argument for " + name(what_name, true));

        this->add_value(parsed_value);
    }
    
    virtual void update_reference(){
        if (this->assign_to_)
        {
            if (!this->is_set() && default_)
                *this->assign_to_ = *default_;
            else if (this->is_set())
                *this->assign_to_ = values_.back();
        }
    }

    virtual void add_value(const T& value){
        values_.push_back(value);
        update_reference();
    }

    void clear() override {
        values_.clear();
        update_reference();
    }

    T* assign_to_;
    std::vector<T> values_;
    std::unique_ptr<T> default_;
};

template <>
inline void Value<std::string>::parse(OptionName what_name, const char* value)
{
    if (strlen(value) == 0)
        throw invalid_option(this, invalid_option::Error::missing_argument, what_name, value, "missing argument for " + name(what_name, true));

    add_value(value);
}

template <>
inline void Value<bool>::parse(OptionName /*what_name*/, const char* value)
{
    bool val =
        ((value != nullptr) && ((strcmp(value, "1") == 0) || (strcmp(value, "true") == 0) || (strcmp(value, "True") == 0) || (strcmp(value, "TRUE") == 0)));
    add_value(val);
}

/// Value option with implicit default value
/**
 * Value option with implicit default value
 * If set, an argument is optional
 * -without argument it carries the implicit default value
 * -with argument it carries the explicit value
 */
template <class T>
class Implicit : public Value<T>
{
public:
    Implicit(const std::string& short_name, const std::string& long_name, const std::string& description, const T& implicit_val, T* assign_to = nullptr): Value<T>(short_name, long_name, description, implicit_val, assign_to){}

    Argument argument_type() const override {
        return Argument::optional;
    }

protected:
    void parse(OptionName what_name, const char* value) override {
        if ((value != nullptr) && (strlen(value) > 0))
            Value<T>::parse(what_name, value);
        else
            this->add_value(*this->default_);
    }
};

/// Value option without value
/**
 * Value option without value
 * Does not require an argument
 * Can be either set or not set
 */
class Switch : public Value<bool>
{
public:
    Switch(const std::string& short_name, const std::string& long_name, const std::string& description, bool* assign_to = nullptr);

    void set_default(const bool& value) = delete;
    Argument argument_type() const override;

protected:
    void parse(OptionName what_name, const char* value) override;
};

/// OptionParser manages all Options
/**
 * OptionParser manages all Options
 * Add Options (Option_Type = Value<T>, Implicit<T> or Switch) with "add<Option_Type>(option params)"
 * Call "parse(argc, argv)" to trigger parsing of the options and to
 * fill "non_option_args" and "unknown_options"
 */
class OptionParser
{
public:
    /// Construct the OptionParser
    /// @param description used for the help message
    explicit OptionParser(std::string description = "");

    /// Destructor
    virtual ~OptionParser() = default;

    /// Add an Option e.g. 'add<Value<int>>("i", "int", "description for the -i option")'
    /// @param T the option type (Value, Switch, Implicit)
    /// @param attribute the Option's attribute (inactive, hidden, required, optional, ...)
    /// @param Ts the Option's parameter
    template <typename T, Attribute attribute, typename... Ts>
    std::shared_ptr<T> add(Ts&&... params){
        static_assert(std::is_base_of<Option, typename std::decay<T>::type>::value, "type T must be Switch, Value or Implicit");
        std::shared_ptr<T> option = std::make_shared<T>(std::forward<Ts>(params)...);

        for (const auto& o : options_)
        {
            if ((option->short_name() != 0) && (option->short_name() == o->short_name()))
                throw std::invalid_argument("duplicate short option name '-" + std::string(1, option->short_name()) + "'");
            if (!option->long_name().empty() && (option->long_name() == (o->long_name())))
                throw std::invalid_argument("duplicate long option name '--" + option->long_name() + "'");
        }
        option->set_attribute(attribute);
        options_.push_back(option);
        return option;
    }

    /// Add an Option e.g. 'add<Value<int>>("i", "int", "description for the -i option")'
    /// @param T the option type (Value, Switch, Implicit)
    /// @param Ts the Option's parameter
    template <typename T, typename... Ts>
    std::shared_ptr<T> add(Ts&&... params){
        return add<T, Attribute::optional>(std::forward<Ts>(params)...);
    }

    /// Parse an ini file into the added Options
    /// @param ini_filename full path of the ini file
    void parse(const std::string& ini_filename);

    /// Parse the command line into the added Options
    /// @param argc command line argument count
    /// @param argv command line arguments
    void parse(int argc, const char* const argv[]);

    /// Delete all parsed options
    void reset();

    /// Produce a help message
    /// @param max_attribute show options up to this level (optional, advanced, expert)
    /// @return the help message
    std::string help(const Attribute& max_attribute = Attribute::optional) const;

    /// Get the OptionParser's description
    /// @return the description as given during construction
    std::string description() const{
        return description_;
    }

    /// Get all options that where added with "add"
    /// @return a vector of the contained Options
    const std::vector<Option_ptr>& options() const{
        return options_;
    }

    /// Get command line arguments without option
    /// e.g. "-i 5 hello" => hello
    /// e.g. "-i 5 -- from here non option args" => "from", "here", "non", "option", "args"
    /// @return vector to "stand-alone" command line arguments
    const std::vector<std::string>& non_option_args() const{
        return non_option_args_;
    }

    /// Get unknown command options
    /// e.g. '--some_unknown_option="hello"'
    /// @return vector to "stand-alone" command line arguments
    const std::vector<std::string>& unknown_options() const{
        return unknown_options_;
    }

    /// Get an Option by it's long name
    /// @param the Option's long name
    /// @return a pointer of type "Value, Switch, Implicit" to the Option or nullptr
    template <typename T>
    std::shared_ptr<T> get_option(const std::string& long_name) const {
        Option_ptr option = find_option(long_name);
        if (!option)
            throw std::invalid_argument("option not found: " + long_name);
        auto result = std::dynamic_pointer_cast<T>(option);
        if (!result)
            throw std::invalid_argument("cannot cast option to T: " + long_name);
        return result;
    }

    /// Get an Option by it's short name
    /// @param the Option's short name
    /// @return a pointer of type "Value, Switch, Implicit" to the Option or nullptr
    template <typename T>
    std::shared_ptr<T> get_option(char short_name) const {
        Option_ptr option = find_option(short_name);
        if (!option)
            throw std::invalid_argument("option not found: " + std::string(1, short_name));
        auto result = std::dynamic_pointer_cast<T>(option);
        if (!result)
            throw std::invalid_argument("cannot cast option to T: " + std::string(1, short_name));
        return result;
    }

protected:
    std::vector<Option_ptr> options_;
    std::string description_;
    std::vector<std::string> non_option_args_;
    std::vector<std::string> unknown_options_;

    Option_ptr find_option(const std::string& long_name) const;
    Option_ptr find_option(char short_name) const;
};

inline std::ostream& operator<<(std::ostream& out, const OptionParser& op)
{
    return out << op.help();
}

/// Base class for an OptionPrinter
/**
 * OptionPrinter creates a help message for a given OptionParser
 */
class OptionPrinter
{
public:
    /// Constructor
    /// @param option_parser the OptionParser to create the help message from
    explicit OptionPrinter(const OptionParser* option_parser) : option_parser_(option_parser){
    }
    /// Destructor
    virtual ~OptionPrinter() = default;

    /// Create a help message
    /// @param max_attribute show options up to this level (optional, advanced, expert)
    /// @return the help message
    virtual std::string print(const Attribute& max_attribute = Attribute::optional) const = 0;

protected:
    const OptionParser* option_parser_;
};

/// Option printer for the console
/**
 * Standard console option printer
 * Creates a human readable help message
 */
class ConsoleOptionPrinter : public OptionPrinter
{
public:
    explicit ConsoleOptionPrinter(const OptionParser* option_parser);
    ~ConsoleOptionPrinter() override = default;

    std::string print(const Attribute& max_attribute = Attribute::optional) const override;

private:
    std::string to_string(Option_ptr option) const;
};
};//!namesapce zcf

#endif//!ZCF_FLAGS_HPP_