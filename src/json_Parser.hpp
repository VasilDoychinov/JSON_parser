// json_Parser.cpp: to parse data in json format
//		- _parse(): checks and generates(loads) a seq of sections. RECURSIVE 
//		  in case of an error: 
//			- returns the sequence as is: possible analisys or,
//			- throws std::range_error() if abrupt ...
//		- (de)activate(): (de)activates a .json descriptor. Used in ~cl_jsonDescr()
//		- iterators:
//			- scope: constructor, operator++
//			- data: construced to be used within cl_SceneDescr -> all checks and resources necessary are
//					to be provided before calling
//		- auxiliary: 
//			- move_to() family: read from the current position until eof or a delimiter found
//			- _f_read() : _read(_lseek())
//

#include <iostream>
using std::cout ;
using std::endl ;

#include <utility>

#include "json_Parser.h"
																		
																		// class cl_jsonDescr
template <typename JCont> bool
cl_jsonDescr<JCont>::activate() &
{
	if (!_jcont.activate())		return false ;

	long addr{} ;
	if (_sections.empty())		_sections = this->_parse(0L, "", 0, addr), _sections.shrink_to_fit() ;
	if (_sections.empty())		{ _jcont.deactivate() ; } // EMPTY: no need of the source ???return false ; }
	
	return true ;
} // cl_jsonDescr activate()


template <typename JCont> jsonSections
cl_jsonDescr<JCont>::_parse(long from_pos, const std::string& name_parent, unsigned char start_ch, long& addr_reached) // recursive
{
	if (_jcont.invalid())	throw std::runtime_error("___ <" + this->id() + "> not active") ;

	static const auto	jc_it_beg{_jcont.begin()} ;			// start of data
	static const auto	jc_it_end{_jcont.end()} ;			// end(): after the last one (the limit)
	static const long	addr_eof{_jcont.distance(jc_it_beg, jc_it_end)} ;

	if (from_pos > addr_eof)	throw std::runtime_error("___ json_Parser: position error (addr: " + std::to_string(from_pos) + ")") ;

	jsonSections	sections{} ;							// fill this
	if (from_pos == addr_eof || addr_eof <= 1)				return sections ;		// End || EMPTY

	s_sectionAtom	w_sec{} ;			// for a section processed
	bool			fl_name{false} ;	// if a name's been reached

	unsigned char	c_buff{} ;			// to read a byte into
	std::string		_wbuff{} ;			// to get text out: like a Section Name
	long			addr{from_pos} ;

	// assert(addr < addr_eof) ;
																		// cout << endl << "___ <" << _fname 
																		// << "> within: [" << addr << ", " 
																		// << addr_eof << "]" ;
	auto	jc_it{jc_it_beg} ; _jcont.advance(jc_it, addr) ;

	static int noname_counter{0} ;
	if (addr == 0L)   noname_counter = 0 ;

	for ( ; addr < addr_eof ; addr++) {
		if ((c_buff = *jc_it) == 0)		break ;
		++jc_it ;

		if (c_buff == '\"') {		// delimiter linked to a name
			// moved after name long	w_addr = addr ;

			try {
				_wbuff = _jcont.string_until_char(jc_it, "\"") ;
			} catch (...) { throw std::runtime_error("___ json_Parser: closing \" missing or read error") ; }

			// position jc_it after the name : part
			long   w_addr = _wbuff.size() + 1 ;
			_jcont.advance(jc_it, w_addr) ;	// move to end of name after "
			w_addr += addr ;        // w_addr = addr + _wbuff.size() + 1 ;
			fl_name = true ;		// indicate: straight away might be a data section ending with closing ]/}

			if (!((jc_it = _jcont.find(jc_it, JSON_NAME_colon)) != jc_it_end))     throw std::runtime_error("___ json_Parser: missing separator") ;
			++jc_it, w_addr++ ;		// just after the name : separator; move on

			// move to opening of a new section or, to closing of an old one
			if (!((jc_it = _jcont.find(jc_it, JSON_all_delimiters)) != jc_it_end)) throw std::runtime_error("___ json_Parser: wrong format") ;
			c_buff = *jc_it, addr = _jcont.distance(jc_it_beg, jc_it) ;
			++jc_it ;
																		// cout << endl << "> found (" << c_buff << ") at: " << addr ; 
																		// << " == o1/o1:" << std::boolalpha << (c_buff == '[' || c_buff == '{') ;
			if (c_buff == JSON_open1 || c_buff == JSON_open2) {
				w_sec.set_section_name(name_parent, _wbuff) ;
				w_sec._addr_start = addr ;

				fl_name = false ;  // clear
																		// cout << endl << ": down for " << w_sec ;
				// recursion
				jsonSections	sect_ancest{this->_parse(++addr, w_sec._name, c_buff, addr)} ;
				// back from recursion
				sections.insert(end(sections), begin(sect_ancest), end(sect_ancest)) ;
				if (addr <= w_sec._addr_start)		throw std::runtime_error("___ json_Parse: position error") ;
				_jcont.advance(jc_it, addr - w_sec._addr_start), w_sec._addr_end = addr ;
																		// cout << endl << " > pushing tree" << w_sec ;
				sections.push_back(std::move(w_sec)), w_sec = s_sectionAtom{} ;
			} else if ((c_buff == JSON_end1 && start_ch == JSON_open1) || (c_buff == JSON_end2 && start_ch == JSON_open2)) {
				if (fl_name) {
					fl_name = false ;
					w_sec.set_section_name(name_parent, _wbuff) ;
					w_sec._addr_start = w_addr, w_sec._addr_end = addr ;
																		// cout << endl << " > pushing leaf" << w_sec ;
					sections.push_back(std::move(w_sec)), w_sec = s_sectionAtom{} ;
				} // now, break UP
				break ;
			} else if (c_buff == JSON_delimiter) {
				fl_name = false ;
				w_sec.set_section_name(name_parent, _wbuff) ;
				w_sec._addr_start = w_addr, w_sec._addr_end = addr ;
				sections.push_back(std::move(w_sec)), w_sec = s_sectionAtom{} ;
			}
		} else if (c_buff == JSON_open1 || c_buff == JSON_open2) {		// opening: go DOWN - but NO NAME														
			w_sec.set_section_name(name_parent, "\0" + std::to_string(noname_counter++)) ; // \0 indicates No-Name ;
			w_sec._addr_start = addr ;
																		// cout << endl << ": down for " << w_sec ;
			jsonSections	sect_ancest{this->_parse(++addr, w_sec._name, c_buff, addr)} ;
			sections.insert(end(sections), begin(sect_ancest), end(sect_ancest)) ;

			if (addr <= w_sec._addr_start)		throw std::runtime_error("___ parse: position error") ;
			_jcont.advance(jc_it, addr - w_sec._addr_start), w_sec._addr_end = addr ;
																		// cout << endl << " > pushing tree" << w_sec ;
			sections.push_back(std::move(w_sec)), w_sec = s_sectionAtom{} ;
		} else if (c_buff == JSON_end2) {   // if closing: go UP
			if (start_ch == JSON_open2)	break ;   // go up
		} else if (c_buff == JSON_end1) {   // closing: go UP
			if (start_ch == JSON_open1)	break ;   // go up
		}
	}

	addr_reached = _jcont.distance(jc_it_beg, jc_it) - 1 ;
	return sections ;	// the so far initiated -> could use it in case of an error as well
} // cl_jsonDescr:: _parse()

template <typename JCont> std::ostream&
cl_jsonDescr<JCont>::show_all_sections(std::ostream& os) const& //,const std::vector<cl_sectionUnit>& ss)
{
	if (_jcont.invalid()) { os << "\n--- <" << this->id() << "> not active" ; return os ; }

	typename JCont::value_type open_ch ;			// consistency check: must match and be either '[]' or '{}'
	typename JCont::value_type close_ch ;			// except for _data fields(scopes): " and ,/}/]

	auto	it{_jcont.begin()} ;
	os << "\n- # of sections found: " << _sections.size() ;
	for (auto& s : _sections) {
		os << "\n--- " ; 
		if (s._name.size() > 0)		os << (s._name[0] == '\0' ? ("-no-name#" + s._name.substr(1)) : s._name) ;
		it = _jcont.begin() ;
		_jcont.advance(it, s._addr_start), open_ch = *it ;
		_jcont.advance(it, s._addr_end - s._addr_start), close_ch = *it ;
		os << open_ch << s._addr_start << ", " << s._addr_end << close_ch ;	// output [open/close]_ch to verify
	}

	return os ;
} // cl_jsonDescr show_all_sections()

template <typename JCont> std::ostream&
cl_jsonDescr<JCont>::show_section(std::ostream& os, const std::string& parent_scope) const& //,const std::vector<cl_sectionUnit>& ss)
{
	if (_jcont.invalid())		{ os << "\n--- <" << this->id() << "> not active" ; return os ; }

	os << "\n- # of sections(total): " << _sections.size() ;
	if (_sections.size() == 0)	return os ;
													// os << " --- <" << parent_scope << ">" ;
	typename JCont::value_type open_ch ;			// consistency check: must match and be either '[]' or '{}'
	typename JCont::value_type close_ch ;			// except for _data fields(scopes): " and ,/}/]

	auto	it{iterator(_sections, parent_scope)} ;
	auto	it_e{it.limit()} ;
	if (!(it != it_e)) { os << " : <" << parent_scope << "> missing" ; return os ; }

	os << " : scanning <" << it.scope() << ">" ;

	for ( ; it != it_e ; ++it) { auto descr_it{_jcont.begin()} ;
		os << "\n--- " << it.id() ;
		_jcont.advance(descr_it, it->_addr_start), open_ch = *(descr_it) ;
		_jcont.advance(descr_it, it->_addr_end - it->_addr_start), close_ch = *(descr_it) ;
		os << open_ch << it->_addr_start << ", " << it->_addr_end << close_ch ;	// output [open/close]_ch to verify
	}

	return os ;
} // cl_jsonDescr show_section()


#ifdef JSON_FILE

std::vector<std::string>
extract_data(cl_jsonDescr& scene,               // the Scene
			 const std::string& scope,          // the Scope
			 const std::string& name,           // variable(s) section
			 int ss)                            // step size of ... in ','s
{
	std::vector<std::string>    values{} ;
	cl_sectionUnit              s_unit{scene.data(scope,name)} ;

	if (s_unit != cl_sectionUnit{})	{
		auto iter = scene.begin_data(s_unit, ss) ;
		auto iter_e = iter.limit() ;

		for ( ; iter != iter_e ; ++iter) {
			// cout << endl << "> scope<" << iter_b.scope() << "> --- section #" << i << ": " << *iter_b ;
			// cout << endl << "--- found #" << i << ": " << *iter ;
			values.push_back(*iter) ;
		}
	}
	return values ;
} // friend cl_jsonDescr() extract_data()
																		// eoc cl_jsonDescr
#endif
// eof json_Parser.hpp