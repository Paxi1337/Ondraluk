/**
 *  This file is part of the debuglib project
 *  Copyright by Christian Ondracek
 *  Contact: coder[at]paxi.at
 *
 *  Logger namespace
 */

#ifndef LOGGER_H
#define LOGGER_H

#define USE_FILE_LOGGER 1
#define USE_TIME_FORMATTER 1
#define LOG_MSG_MAX_SIZE 255
#define _DEBUG 1



#include <iostream>
#include <set>
#include <stdarg.h>

#ifdef _WIN32
	#include <malloc.h>
	#include <Windows.h>
	// disable unsecure deprecation
	#pragma warning(disable: 4996)
#else
	#include <alloca.h>
	#include <cstdarg>
#endif

#ifdef USE_FILE_LOGGER
	#include <fstream>
	#include <memory>
	#include <utility>
	#include <cstdio>
#endif

#include "Logdispatch.h"

#ifdef _DEBUG

extern debuglib::logdispatch::LoggerManager LoggerMgr;

#ifdef _WIN32
#define LOG(channel, loglevel, formated_message, ...) \
	debuglib::logdispatch::LoggerMgr.log(channel, loglevel, formated_message, __VA_ARGS__);

#else
#define LOG(channel, loglevel, formated_message, ...) \
	debuglib::logdispatch::LoggerMgr.log(channel, loglevel, formated_message, ##__VA_ARGS__);

#endif


#define REGISTER_LOG_CHANNEL(channel) \
	debuglib::logdispatch::LoggerMgr.registerChannel(channel);

#else // retail
	#define LOG(channel, loglevel, formated_message, ...) \
		(void) channel; \
		(void) loglevel; \
		(void) formated_message;

	#define REGISTER_LOG_CHANNEL(channel) \
		(void) channel;
#endif

namespace debuglib 
{
	namespace logger 
	{
		// log levels
		static const int UNDEFINED = 0;
		static const int DEBUG = 1;
		static const int INFO = 2;
		static const int WARN = 3;
		static const int ERR = 4;
		static const int FATAL_ERR = 5;

		/**
		 * Used within all Filter Policies to encapsulate the information from a log message
		 */
		struct FilterAttributes {
			FilterAttributes() : mChannel(UNDEFINED), mLoglevel(UNDEFINED) {}
			FilterAttributes(int channel, int loglevel) : mChannel(channel), mLoglevel(loglevel) {}

			int mChannel;
			int mLoglevel;
		};

		#pragma region FilterPolicies
			/**
			 * Filters by verbosity.
			 */
			struct LogLevelFilter {
				LogLevelFilter() : mVerbosity(UNDEFINED) {}
				explicit LogLevelFilter(int verbosity) : mVerbosity(verbosity) {}
				LogLevelFilter(LogLevelFilter&& other) : mVerbosity(other.mVerbosity) {}

				bool filter(const FilterAttributes& attrs) const {
					if(attrs.mLoglevel == UNDEFINED) {
						return false;
					} else if(attrs.mLoglevel >= mVerbosity) {
						return true;
					} else {
						return false;
					}
				}

				int mVerbosity;
			};

			/**
			 * Filters by channel.
			 */
			struct ChannelFilter {
				ChannelFilter() : mChannel(UNDEFINED) {}
				explicit ChannelFilter(int channel) : mChannel(channel) {}
				ChannelFilter(ChannelFilter&& other) : mChannel(other.mChannel) {}

				bool filter(const FilterAttributes& attrs) const {
					if(attrs.mChannel == mChannel) {
						return true;
					} else {
						return false;
					}
				}

				int mChannel;
			};

			/**
			 * Filters none.
			 */
			struct NoFilter {
				NoFilter() {}
				NoFilter(NoFilter&&) {}

				bool filter(const FilterAttributes& criteria) const {
					(void)criteria;
					return true;
				}

			private:

			};
		#pragma endregion FilterPolicies


		#pragma region FormatPolicies
			/**
			 * Simple formatter.
			 */
			struct SimpleFormatter {
				void format(const char* formatted_message, char* dest, va_list args, size_t size) const {
					int bytes_written = vsnprintf(dest, (size-2), formatted_message, args);
					dest[bytes_written] = '\n';
					dest[bytes_written+1] = '\0';
				}
			};

#ifdef USE_TIME_FORMATTER
			/**
			 * Time formatter.
			 */
			struct TimeFormatter {
				void format(const char* formatted_message, char* dest, va_list args, size_t size) const {
				}
			};
#endif
		#pragma endregion FormatPolicies


		#pragma region OutputPolicies
			/**
			 * Outputting to Visual Studio Log console.
			 * Only available within Windows.
			 */
			#ifdef _WIN32
				// move ctor is obsolete..

				struct VSOutputter {
					void out(const char* msg) const {
						OutputDebugStringA(msg);
					}
				};
			#endif
			
			/**
			 * Outputting to cout.
			 */
			struct ConsoleOutputter {
				// move ctor is obsolete..
				
				void out(const char* msg) const {
					printf("%s", msg);
					fflush(stdout);
				}
			};

			/**
			 * Outputting to a file.
			 *
			 * @remark 
			 *
			 *
			 */
			#ifdef USE_FILE_LOGGER


			struct FileOutputter {
				// c_tor
				FileOutputter() {}
				
				// move constructor
				FileOutputter(FileOutputter&& other) {
					std::swap(other.mStream, mStream);
				}

				// c_tor
				explicit FileOutputter(const char* fname) : mStream(std::make_shared<std::fstream>(fname, std::fstream::out)) {
				
				}
				
				void out(const char* msg) const {
					*mStream << msg;
				}

				mutable std::shared_ptr<std::fstream> mStream;

			private:
				FileOutputter(const FileOutputter&);
				FileOutputter& operator=(const FileOutputter& other);
			};

			#endif

		#pragma endregion OutputPolicies


		/**
		 * Base class of all instantiated loggers.
		 * Mainly used for dispatching loggers within logdispatch namespace.
		 */
		class LoggerBase {
		public:
			// see comments in LoggerImpl
			virtual void log(int channel, int loglevel, const char* formated_message, va_list list) const = 0;
			virtual ~LoggerBase(void) { }
		};

		/**
		 * Implementation of the logger class.
		 * Uses policies for generic usage and extension.
		 */
		template <class Filter, class Formatter, class Outputter>
		class LoggerImpl : public LoggerBase {
		public:

		  /**
			* Constructor
			*
			* Every instantiated logger is automatically added to the logdispatch list.
			*
			* @remark The temporary object is bound to an rvalue reference and will call the move contructor if available.
			*		  If there is no move constructor compiler may use some kind of copy elision or call copy constructor.
			*
			* @param[in] The filter policy.
			* @param[in] The formatter policy.
			* @param[in] The outputter plicy.
			*/
			
			LoggerImpl(Filter filter = Filter(), Formatter formatter = Formatter(), Outputter outputter = Outputter());

			/**
			 * Destructor
			 *
			 * Every deleted logger will removed itself from the logdispatch list.
			 */
			~LoggerImpl();

			/**
			 * Output a message on this logger.
			 *
			 * @remark Every log message will be appended by a \n
			 *
			 * @param[in] channel The channel for the message.
			 * @param[in] loglevel The log level.
			 * @param[in] formated_message The message itself as formatted string; f.e: Sum = %d
			 * @param[in] list Variable arguments passed in by the log macro.
			 *
			 * @return void
			 */
			void log(int channel, int loglevel, const char* formated_message, va_list list) const;
		private:
			Filter mFilter;
			Formatter mFormatter;
			Outputter mOutputter;
		};

		
		template <class Filter, class Formatter, class Outputter>
		LoggerImpl<Filter, Formatter, Outputter>::LoggerImpl(Filter filter, Formatter formatter, Outputter outputter) : 
			mFilter(std::move(filter)), mFormatter(std::move(formatter)), mOutputter(std::move(outputter)) {
				debuglib::logdispatch::LoggerMgr.addLogger(this);
		}

		
		template <class Filter, class Formatter, class Outputter>
		LoggerImpl<Filter, Formatter, Outputter>::~LoggerImpl() {
				debuglib::logdispatch::LoggerMgr.removeLogger(this);
		}

		template <class Filter, class Formatter, class Outputter>
		void LoggerImpl<Filter, Formatter, Outputter>::log(int channel, int loglevel, const char* formated_message, va_list list) const {
			
			FilterAttributes attrsFilter(channel, loglevel);
			size_t s = 0;

			if(mFilter.filter(attrsFilter)) {

#ifdef _WIN32
				s = _vscprintf(formated_message, list) + 2;
				char* tmp = static_cast<char*>(_malloca(s));
#else
				s = vsnprintf(NULL, 0, formated_message, list) + 2;
				char* tmp = static_cast<char*>(__builtin_alloca(s));
#endif

				mFormatter.format(formated_message, tmp, list, s);
				mOutputter.out(tmp);
			}
		}

		typedef LoggerImpl<ChannelFilter, SimpleFormatter, ConsoleOutputter> SimpleChannelConsoleLogger;
		typedef LoggerImpl<LogLevelFilter, SimpleFormatter, ConsoleOutputter> SimpleLogLevelConsoleLogger;
		typedef LoggerImpl<NoFilter, SimpleFormatter, ConsoleOutputter> ConsoleLogger;
		typedef LoggerImpl<NoFilter, SimpleFormatter, FileOutputter> FileLogger;
		typedef LoggerImpl<NoFilter, TimeFormatter, FileOutputter> TimeFormattedFileLogger;

#ifdef _WIN32
		typedef LoggerImpl<ChannelFilter, SimpleFormatter, VSOutputter> SimpleChannelVSLogger;
		typedef LoggerImpl<LogLevelFilter, SimpleFormatter, VSOutputter> SimpleLogLevelVSLogger;
#endif

	}// logger end
}// debuglib end

#endif
