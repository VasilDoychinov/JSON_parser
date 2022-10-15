// json_File.cpp: follows json_File.h
//

#include "json_File.h"

																			// class cl_jsonFile
bool
cl_jsonFile::activate() &
{
	if (_fh == FILE_Not_Active) {
		errno_t err = _sopen_s(&_fh, _fname.c_str(), _O_BINARY | _O_RDONLY,
							   _SH_DENYRW, _S_IREAD | _S_IWRITE) ;
		if (err != 0) { _fh = FILE_Not_Active ; return false ; }
	}
	return true ;
} // cl_jsonFile activate()
																			// eoc cl_jsonFile
																			// class cl_iteratorFile
cl_iteratorFile
cl_iteratorFile::_find_IF(const std::string& list_delim) &
{
	auto	it_end{cl_iteratorFile(_fh, _lseek(_fh, 0L, SEEK_END))} ;

	auto	addr{this->tell()}, addr_eof{it_end.tell()} ;
	if (_lseek(_fh, addr, SEEK_SET) < 0L)		return it_end ;

	for ( ; addr < addr_eof ; addr += sizeof(value_type)) {
		if (_read(_fh, static_cast<value_type *>(&_buff), sizeof(_buff)) != sizeof(_buff))	break ;	// error or not found
		if (list_delim.find(_buff) != std::string::npos)		return cl_iteratorFile(_fh, addr, _buff) ;
	}

	_buff = value_type{} ;
	return it_end ;			// error or not found
} // cl_jsonFile find()

std::string
cl_iteratorFile::_such_IF(const std::string& list_delim) &
{
	std::string		str{} ;
	_buff = value_type{} ;
	
	auto	it_end{cl_iteratorFile(_fh, _lseek(_fh, 0L, SEEK_END))} ;
	auto	addr{this->tell()}, addr_eof{it_end.tell()} ;

	if (!(_lseek(_fh, addr, SEEK_SET) < 0L)) {	// positioned OK
		for ( ; addr < addr_eof ; addr += sizeof(value_type)) {
			if (_read(_fh, static_cast<value_type *>(&_buff), sizeof(_buff)) != sizeof(_buff))	break ;
			if (list_delim.find(_buff) != std::string::npos)	return str ;
			str.push_back(_buff) ;
		}
	}

	return std::string{} ; // throw std::runtime_error("") ;	// error
} // cl_jsonFile::string_until_char()
																			// eoc cl_iteratorFile
// eof json_File.cpp