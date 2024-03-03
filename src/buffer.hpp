#pragma once

#include "stdlibs.hpp"

struct Buffer {
    bool empty() const {
	return data.empty() ||
	    (!last_block_size && &data.front() == &data.back());
    }
    bool eol_at_end() const {
	if(empty()) return false;
	else if(last_block_size)
	    return data.back()[last_block_size - 1] == '\n';
	else {
	    auto p = data.end();
	    --p;
	    --p;
	    return p->at(p->size() - 1) == '\n';
	}
    }
    typedef std::array<char, 4096> Block;
    std::list<Block> data;
    int last_block_size = 0;
};

inline std::ostream & operator << (std::ostream &out, const Buffer &buffer) {
    auto p = buffer.data.begin(), e = buffer.data.end();
    if(p != e)
	for(;;) {
	    auto m = p++;
	    if(p == e) {
		out.write(m->data(), buffer.last_block_size);
		break;
	    }
	    else out.write(m->data(), m->size());
	}
    return out;
}

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * tab-width: 4
 * End:
 */
