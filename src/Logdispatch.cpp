/**
 *  this file is part of the debuglib project
 *  Copyright by coder@paxi.at
 */


#include "../includes/Logger.h"
#include <algorithm>
#include <cstdarg>

namespace debuglib 
{
	namespace logdispatch 
	{
		LoggerManager LoggerMgr;

		LoggerManager::LoggerManager() {
			mChannels.insert(1);
		}

		LoggerManager::~LoggerManager() {
			// no need to clear mChannels since the memory will be freed anyway after the LoggerManager is destructed and deleted
		}

		void LoggerManager::addLogger(debuglib::logger::LoggerBase* l) {
			mLoggers.push_back(l);
		}

		void LoggerManager::removeLogger(debuglib::logger::LoggerBase* l) {
			std::remove(mLoggers.begin(), mLoggers.end(), l);
		}

		int LoggerManager::size() {
			return mLoggers.size();
		}

		void LoggerManager::log(int channel, int loglevel, const char* formated_message, ...) {
			
			if(mChannels.find(channel) != mChannels.end()) {

				va_list list;
				va_start(list, formated_message);

				if(size()) {
					for(std::vector<debuglib::logger::LoggerBase*>::const_iterator it = mLoggers.cbegin(); it != mLoggers.cend(); ++it) {
						debuglib::logger::LoggerBase* tmp = *it;
			
						tmp->log(channel, loglevel, formated_message, list);
					}
				}

				va_end(list);
			}
		}

		void LoggerManager::registerChannel(int channel) {
			std::pair<std::set<int>::iterator,bool> ret = mChannels.insert(channel);
			
			// element already existed
			if(ret.second == false) {
				LOG(1, debuglib::logger::ERR, "Channel %d already taken", channel);
			}
		}
	}
}