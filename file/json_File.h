// json_File.h: type to handle data in JSON format contained in files
//      Must provide:
//          - constructors, destructors as
//          - bool activate(): if succesfully activated
//          - void deactivate(): deactivate / destroy / release resources
//          - bool invalid(): if the container is invalid
//          - id(): JCont's identifier as a const string &
//          
//          The rest of required follow the forward constant_iterator model/support
//          - begin(): forward iterator to the start of JCont
//          - end():   forward iterator: jsut after the last character of
//          - .advance(iterator&, distance): follows std::advance()
//          - unsigned char operator *(iterator): 
//          - iterator& operator ++(): 
//          - std::string string_until_char(iterator, string of chars): returns the string
//            from iterator to any of "string of chars";
//          - find(iterator, string of chars): iterator to the 1st of "string of chars" found or,
//            end() if none
//          - .distance(start, end): follow std::distance()
//

#include <iostream>

#ifndef _DEFS_JSON_FILE_
#define _DEFS_JSON_FILE_

#include <string>

#include <stdio.h>          // I/O support
#include <io.h>
#include <fcntl.h>
#include <share.h>

#include <assert.h>

class cl_iteratorFile {
    public:
        using iterator_category = std::forward_iterator_tag ;
        using value_type = typename unsigned char ;

    private:
        int         _fh ;   // low level I/O File Handle
        long        _addr ;
        value_type  _buff ;

    public:
        explicit cl_iteratorFile(int fh, long a = 0L, value_type ch = value_type{}) : _fh{fh}, _addr{a}, _buff{ch} { assert(fh >= 0) ; }
        // all specials  = default 

        decltype(auto) tell() const& { return _addr ; }

        bool operator !=(const cl_iteratorFile& rs) const& { return _addr != rs._addr || _fh != rs._fh ; }
        value_type operator *() { if (_buff != value_type{})  return _buff ;
            _lseek(_fh, _addr, SEEK_SET) ;
            if (_read(_fh, static_cast<value_type *>(&_buff), sizeof(_buff)) != sizeof(_buff))   return value_type{} ;
            return _buff ;
        }

        // movements
        cl_iteratorFile& operator ++() { _addr += sizeof(value_type), _buff = value_type{} ; return *this ; }

        cl_iteratorFile _find_IF(const std::string& list_delim) & ;
        std::string     _such_IF(const std::string& list_delim) & ;
        
        friend long _distance_IF(const cl_iteratorFile& b, const cl_iteratorFile& e) { return e._addr - b._addr ; }
        friend void _advance_IF(cl_iteratorFile& it, long hops) { it._addr += hops, it._buff = value_type{} ; }

        friend std::ostream& operator <<(std::ostream& os, const cl_iteratorFile& it) {
            os << "itFile{f: " << it._fh << ", a: " << it._addr << ", b: " << it._buff << "}" ;
            return os ;
        }
}; // class cl_iteratorFile

class cl_jsonFile {
    public:
        using   iterator = cl_iteratorFile ;
        using   value_type = iterator::value_type ;

    private:
        static constexpr int FILE_Not_Active{-1} ;

        std::string     _fname{} ;
        int             _fh ;       // low level I/O File handle

    public:
        explicit cl_jsonFile(const std::string& name) : _fname{name}, _fh{FILE_Not_Active} {}
        virtual ~cl_jsonFile() { this->deactivate() ; }

        cl_jsonFile(cl_jsonFile&&) = default ;
        cl_jsonFile& operator =(cl_jsonFile&&) = default ;
        cl_jsonFile(const cl_jsonFile&) = delete ;
        cl_jsonFile& operator =(const cl_jsonFile&) = delete ;

        const std::string& id() const& { return _fname ; } 

        bool activate() & ;
        void deactivate() & { if (_fh != -1) { _close(_fh), _fh = FILE_Not_Active ; } }
        bool invalid() const& { return _fh == FILE_Not_Active ; }

        friend class cl_iteratorFile ;

        // iterator support
        iterator begin() const& { return iterator(_fh) ; }
        iterator end() const& { return iterator(_fh, _lseek(_fh, 0L, SEEK_END)) ; }

        decltype(auto) distance(const iterator b, const iterator e) const& { return _distance_IF(b, e) ; }
        decltype(auto) advance(cl_iteratorFile& it, long hops) const& { return _advance_IF(it, hops) ; }

        cl_iteratorFile find(cl_iteratorFile pos, const std::string& list_delim) & { return pos._find_IF(list_delim) ; }
        std::string string_until_char(cl_iteratorFile pos, const std::string& l_d) & { return pos._such_IF(l_d) ; }
}; // class cl_jsonFile

_DEFS_JSON_FILE_
#endif
// eof json_File.h