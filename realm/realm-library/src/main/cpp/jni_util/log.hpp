/*
 * Copyright 2016 Realm Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef REALM_JNI_UTIL_LOG_HPP
#define REALM_JNI_UTIL_LOG_HPP

#include <jni.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "io_realm_log_LogLevel.h"

#include "realm/util/logger.hpp"
#include "util/format.hpp"

// FIXME: env is not needed any more. Will remove it in another PR.
#define TR_ENTER(env) \
    if (realm::jni_util::Log::s_level <= realm::jni_util::Log::trace) { \
        realm::jni_util::Log::t(" --> %1", __FUNCTION__); \
    }
#define TR_ENTER_PTR(env, ptr) \
    if (realm::jni_util::Log::s_level <= realm::jni_util::Log::trace) { \
        realm::jni_util::Log::t(" --> %1 %2" PRId64, __FUNCTION__, static_cast<int64_t>(ptr)); \
    }

namespace realm {

namespace jni_util {

class JniLogger;

// This is built for Realm logging, bother for Java and native side.
// There are multiple loggers can be registered. All registered loggers will receive the same log events.
class Log {
public:
    enum Level {
        all = io_realm_log_LogLevel_ALL,
        trace = io_realm_log_LogLevel_TRACE,
        debug = io_realm_log_LogLevel_DEBUG,
        info = io_realm_log_LogLevel_INFO,
        warn = io_realm_log_LogLevel_WARN,
        error = io_realm_log_LogLevel_ERROR,
        fatal = io_realm_log_LogLevel_FATAL,
        off = io_realm_log_LogLevel_OFF
    };

    // Add & Remove a Java Logger. An Java logger needs to be implemented from io.realm.log.Logger interface.
    void add_java_logger(JNIEnv* env, const jobject java_logger);
    void remove_java_logger(JNIEnv* env, const jobject java_logger);

    void add_logger(std::shared_ptr<JniLogger> logger);
    void remove_logger(std::shared_ptr<JniLogger> logger);

    void clear_loggers();

    void set_level(Level level);
    inline Level get_level() {
        return s_level;
    };

    void log(Level level, const char* tag, jthrowable throwable, const char* stacktrace,
            const char* message);

    inline void log(Level level, const char* tag, const char* message)
    {
        log(level, tag, nullptr, nullptr, message);
    }

    // Helper functions for logging with REALM_JNI tag.
    inline static void t(const char* message)
    {
        shared().log(error, REALM_JNI_TAG, nullptr, nullptr, message);
    }
    inline static void d(const char* message)
    {
        shared().log(error, REALM_JNI_TAG, nullptr, nullptr, message);
    }
    inline static void i(const char* message)
    {
        shared().log(error, REALM_JNI_TAG, nullptr, nullptr, message);
    }
    inline static void w(const char* message)
    {
        shared().log(error, REALM_JNI_TAG, nullptr, nullptr, message);
    }
    inline static void e(const char* message)
    {
        shared().log(error, REALM_JNI_TAG, nullptr, nullptr, message);
    }
    inline static void f(const char* message)
    {
        shared().log(error, REALM_JNI_TAG, nullptr, nullptr, message);
    }

    template<typename... Args>
    inline static void t(const char* fmt, Args&&... args)
    {
        shared().log(trace, REALM_JNI_TAG, nullptr, nullptr, _impl::format(fmt, {_impl::Printable(args)...}).c_str());
    }
    template<typename... Args>
    inline static void d(const char* fmt, Args&&... args)
    {
        shared().log(debug, REALM_JNI_TAG, nullptr, nullptr, _impl::format(fmt, {_impl::Printable(args)...}).c_str());
    }
    template<typename... Args>
    inline static void i(const char* fmt, Args&&... args)
    {
        shared().log(info, REALM_JNI_TAG, nullptr, nullptr, _impl::format(fmt, {_impl::Printable(args)...}).c_str());
    }
    template<typename... Args>
    inline static void w(const char* fmt, Args&&... args)
    {
        shared().log(warn, REALM_JNI_TAG, nullptr, nullptr, _impl::format(fmt, {_impl::Printable(args)...}).c_str());
    }
    template<typename... Args>
    inline static void e(const char* fmt, Args&&... args)
    {
        shared().log(error, REALM_JNI_TAG, nullptr, nullptr, _impl::format(fmt, {_impl::Printable(args)...}).c_str());
    }
    template<typename... Args>
    inline static void f(const char* fmt, Args&&... args) {
        shared().log(fatal, REALM_JNI_TAG, nullptr, nullptr, _impl::format(fmt, {_impl::Printable(args)...}).c_str());
    }

    // Get the shared Log instance.
    static Log& shared();

    // public & static for reading faster. For TR_ENTER check.
    // Accessing to this var won't be thread safe and it is not necessary to be. Changing log level concurrently
    // won't be a critical issue for commons cases.
    static Level s_level;
private:
    Log();

    std::vector<std::shared_ptr<JniLogger>> m_loggers;
    std::mutex m_mutex;
    // Log tag for generic Realm JNI.
    static const char* REALM_JNI_TAG;

};

// Base Logger class.
class JniLogger {
protected:
    JniLogger();
    // Used by JavaLogger.
    JniLogger(bool is_java_logger);
    // Indicate if this is a wrapper for Java Logger class. See JavaLogger
    bool m_is_java_logger;

protected:
    // Overwrite this method to handle the log event.
    // throwable is the Throwable passed from Java which could be null.
    // statcktrace is the stracktrace string parsed from throwable by the Java layer which could be an empty string.
    virtual void log(Log::Level level, const char* tag, jthrowable throwable, const char* stacktrace,
            const char* message) = 0;
    friend class Log;
};

// Implement this function to return the default logger which will be registered during initialization.
extern std::shared_ptr<JniLogger> get_default_logger();

class CoreLoggerBridge : public realm::util::RootLogger {
public:
    void do_log(Logger::Level, std::string msg) override;
    static CoreLoggerBridge& shared();

private:
    CoreLoggerBridge() {};
    // Log tag for Realm core & sync.
    static const char* TAG;
};

} // namespace jni_util
} // namespace realm

#endif // REALM_JNI_UTIL_LOG_HPP