#pragma once

#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/keywords/format.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/attributes/attribute.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/smart_ptr/make_shared_object.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/utility/manipulators/to_log.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

#include <fstream>

namespace logger
{
    enum class severity_level
    {
        debug,
        info,
        warning,
        error,
        fatal
    };

    inline auto& operator<<(std::ostream& stream, severity_level level)
    {
        const char* strings[] = {"debg", "info", "warn", "errr", "ftal"};
        if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        {
            stream << strings[static_cast<int>(level)];
        }
        else
        {
            stream << static_cast< int >(level);
        }
        return stream;
    }

    struct severity_tag;
    inline auto& operator<< (boost::log::formatting_ostream& strm,
                             boost::log::to_log_manip< severity_level, severity_tag > const& manip)
    {
        static const char* strings[] = {"debg", "info", "warn", "errr", "ftal"};

        severity_level level = manip.get();
        if (static_cast< std::size_t >(level) < sizeof(strings) / sizeof(*strings))
        {
            strm << strings[static_cast<int>(level)];
        }
        else
        {
            strm << static_cast< int >(level);
        }
        return strm;
    }

    inline auto _internal_console_formatter(const boost::log::record_view& rec, boost::log::formatting_ostream& strm)
    {
        using namespace boost::log;
        auto severity = rec.attribute_values()[boost::log::aux::default_attribute_names::severity()].
                                               extract<severity_level>();
        if (severity)
        {
            switch (severity.get())
            {
                case severity_level::debug:
                    break;
                case severity_level::info:
                    strm << "\033[34m";
                    break;
                case severity_level::warning:
                    strm << "\033[33m";
                    break;
                case severity_level::error:
                    strm << "\033[31m";
                    break;
                case severity_level::fatal:
                    strm << "\033[1;31m";
            }
        }

        strm <<  extract< unsigned int >("LineID", rec) << ": ";
        if (auto timestamp = boost::log::extract< boost::posix_time::ptime >("TimeStamp", rec))
        {
            std::tm ts = boost::posix_time::to_tm(*timestamp);
            char buf[128];
            if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts) > 0)
            {
                strm << "[" << buf << "]";
            }
        }
        strm << "[" << severity << "] " << rec[expressions::smessage];
        if (severity)
        {
            strm << "\033[0m";
        }
    }

    auto _internal_file_formatter( boost::log::record_view const& rec, boost::log::formatting_ostream& strm)
    {
        using namespace boost::log;
        auto severity = rec.attribute_values()[boost::log::aux::default_attribute_names::severity()].
                                               extract<severity_level>();

        strm <<  extract< unsigned int >("LineID", rec) << ": ";
        if (auto timestamp = boost::log::extract< boost::posix_time::ptime >("TimeStamp", rec))
        {
            auto ts = boost::posix_time::to_tm(*timestamp);

            char buf[128];
            if (std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts) > 0)
            {
                strm << "[" << buf << "]";
            }
        }
        strm << "[" << severity << "] " << rec[expressions::smessage];
    }

    inline void _internal_init_console_sink(severity_level lvl)
    {
        using namespace boost::log;
        typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
        boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();
        boost::shared_ptr< std::ostream > stream(&std::clog, boost::null_deleter());

        sink->locked_backend()->add_stream(stream);
        sink->set_formatter(&_internal_console_formatter);
        sink->set_filter(expressions::attr< severity_level >("Severity") >= lvl);
        boost::log::core::get()->add_sink(sink);
    }

    inline void _internal_init_file_sink(severity_level lvl, const std::string& file_prefix)
    {
        using namespace boost::log;
        typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
        boost::shared_ptr< text_sink > sink = boost::make_shared< text_sink >();

        sink->locked_backend()->add_stream(boost::make_shared< std::ofstream >(file_prefix + "_%N.log"));
        sink->set_formatter(&_internal_file_formatter);
        sink->set_filter(expressions::attr< severity_level >("Severity") >= lvl);
        boost::log::core::get()->add_sink(sink);
    }

    inline void init(const std::string& file_prefix, severity_level lvl)
    {
        using namespace boost::log;
        register_simple_formatter_factory<severity_level, char>("Severity");

        _internal_init_console_sink(lvl);
        _internal_init_file_sink(lvl, file_prefix);
        add_common_attributes();
    }
    template <severity_level level, typename ...Args>
    auto log(const Args& ...args)
    {
        boost::log::sources::severity_logger<severity_level> lg;
        boost::log::record rec = lg.open_record(boost::log::keywords::severity = level);
        if (rec)
        {
            boost::log::record_ostream strm(rec);
            (strm << ... << args);
            strm.flush();
            lg.push_record(boost::move(rec));
        }
    }

    template <typename ...Args>
    void debug(const Args& ...args)
    {
        log<severity_level::debug>(args...);
    }

    template <typename ...Args>
    void info(const Args& ...args)
    {
        log<severity_level::info>(args...);
    }

    template <typename ...Args>
    void warning(const Args& ...args)
    {
        log<severity_level::warning>(args...);
    }

    template <typename ...Args>
    void error(const Args& ...args)
    {
        log<severity_level::error>(args...);
    }

    template <typename ...Args>
    void fatal(const Args& ...args)
    {
        log<severity_level::fatal>(args...);
    }
}