/** 
 * @copyright Copyright © 2020-2024 code by zhaoj
 * 
 * LICENSE
 * 
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

 /**
 * @author zhaoj 286897655@qq.com
 * @brief 
 */

#include "zcf/log/zcf_log.h"
#include "zcf/zcf_sys.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/callback_sink.h>

namespace zcf{
static const char kLogDir[] = "log";
static const char kDebugLogPattern[] = "[%Y-%m-%d %H:%M:%S.%e][%P/%t]<%L~%s>:%v";
static const char kInfoLogPattern[] = "[%Y-%m-%d %H:%M:%S.%e][%P/%t]<%L>:%v";
static const char kRotating[] = "rotating_file_sink";
static constexpr size_t kRotatingSize = 1024*1024*300;//300 M
static size_t kMaxRotating = 10;// 10个文件

static std::shared_ptr<logger> default_logger = nullptr;

std::shared_ptr<logger> logger::getLogger(){
    if(default_logger){
        return default_logger->shared_from_this();
    }
    return nullptr;
}

std::shared_ptr<logger> logger::create_defaultLogger(){
    default_logger = std::make_shared<logger>();
    spdlog::drop_all();

    default_logger->spdlogger = spdlog::create<spdlog::sinks::stdout_color_sink_mt>("stdout_color_sink");
    default_logger->spdlogger->set_pattern(kDebugLogPattern);
    default_logger->spdlogger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(default_logger->spdlogger);

    return default_logger;
}

std::shared_ptr<logger> logger::create_rotateFileLogger(const std::string& logfilename){
    default_logger = std::make_shared<logger>();
    spdlog::drop_all();

    char buf[1024]={0};
    std::string path = sys::getExeDir();
    sprintf(buf,"%s%s/%s",path.c_str(),kLogDir,logfilename.c_str());

    default_logger->spdlogger = spdlog::create<spdlog::sinks::rotating_file_sink_mt>(kRotating,buf,kRotatingSize,kMaxRotating);
    default_logger->spdlogger->set_pattern(kInfoLogPattern);
    default_logger->spdlogger->flush_on(spdlog::level::info);

    spdlog::set_default_logger(default_logger->spdlogger);

    return default_logger;
}

std::shared_ptr<logger> logger::create_asyncRotateFileLogger(const std::string& logfilename){
    throw std::runtime_error("not support async logger now");
}

template<typename Mutex>
class custom_sink final : public spdlog::sinks::base_sink<Mutex>{
public:
    explicit custom_sink(log_callback&& callback): 
        callback_(std::move(callback)){}

protected:
    void sink_it_(const spdlog::details::log_msg &msg) override{
        spdlog::memory_buf_t formatted;
        spdlog::sinks::base_sink<Mutex>::formatter_->format(msg, formatted);

        callback_(fmt::to_string(formatted));
    }

    void flush_() override{};

private:
    log_callback callback_;
};

using custom_call_back_sink = custom_sink<std::mutex>;

std::shared_ptr<logger> logger::create_callBaclLogger(log_callback&& callback){
    default_logger = std::make_shared<logger>();
    spdlog::drop_all();

    default_logger->spdlogger = spdlog::create<custom_call_back_sink>("callback_sink",std::move(callback));
    default_logger->spdlogger->set_pattern(">>>>>>>>> %v <<<<<<<<<");
    spdlog::set_default_logger(default_logger->spdlogger);

    return default_logger;
}

void logger::setLevel(log_level level){
    if(!spdlogger)
        return;

    switch (level){
        case log_level::LOGLEVEL_TRACE:
            spdlogger->set_level(spdlog::level::trace);
            break;
        case log_level::LOGLEVEL_DEBUG:
            spdlogger->set_level(spdlog::level::debug);
            break;
        case log_level::LOGLEVEL_INFO:
            spdlogger->set_level(spdlog::level::info);
            break;
        case log_level::LOGLEVEL_WARN:
            spdlogger->set_level(spdlog::level::warn);
            break;
        case log_level::LOGLEVEL_ERROR:
            spdlogger->set_level(spdlog::level::err);
            break;
    default:
        break;
    }
}
};//!namespace zcf