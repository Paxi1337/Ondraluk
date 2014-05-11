/**
 *  this file is part of the debuglib project
 *  Copyright by coder@paxi.at
 *  logdisptach namespace is used to manage all loggers
 */

#ifndef LOGDISPATCH_H
#define LOGDISPATCH_H

#include <vector>
#include <set>

// forward declarations
namespace debuglib {
	namespace logger {
		class LoggerBase;
		
		template <class Filter, class Formatter, class Outputter>
		class LoggerImpl;
	}
}

namespace debuglib 
{
	namespace logdispatch 
	{
		class LoggerManager {
			// declaring LoggerImpl as friend
			template <class Filter, class Formatter, class Outputter>
			friend class debuglib::logger::LoggerImpl;
		
		public:
			LoggerManager();
			~LoggerManager();

			void log(int channel, int loglevel, const char* formated_message, ...);
			void registerChannel(int channel);
			int size();

		private:
			void addLogger(debuglib::logger::LoggerBase*);
			void removeLogger(debuglib::logger::LoggerBase*);

			std::set<int> mChannels;
			std::vector<debuglib::logger::LoggerBase*> mLoggers;
		};

		extern debuglib::logdispatch::LoggerManager LoggerMgr;
	}
}

#endif