#pragma once

#include <iostream>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/process.hpp>
#include <boost/dll.hpp>
#include <boost/asio.hpp>

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

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
