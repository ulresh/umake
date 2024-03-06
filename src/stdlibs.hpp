#pragma once

#include <iostream>
#include <thread>
#include <map>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/dll.hpp>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using std::cout;
using std::cerr;
using std::endl;
using std::flush;
namespace fs = boost::filesystem;
namespace bp = boost::process;
using boost::system::error_code;
namespace ai = boost::asio;
namespace ph = boost::asio::placeholders;
using boost::asio::io_service;
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::posix_time::second_clock;
using boost::posix_time::microseconds;
using boost::posix_time::milliseconds;
using boost::posix_time::seconds;
using boost::posix_time::minutes;
using boost::posix_time::time_duration;
using boost::posix_time::from_time_t;

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
