// json_Params.h: basic types, constants, etc
//

#ifndef _DEFS_JSON_PARAMETERS_
#define _DEFS_JSON_PARAMETERS_

constexpr unsigned char JSON_delimiter = ',' ;
constexpr unsigned char JSON_open1 = '[' ;
constexpr unsigned char JSON_open2 = '{' ;
constexpr unsigned char JSON_end1 = ']' ;
constexpr unsigned char JSON_end2 = '}' ;
constexpr auto JSON_NAME_colon = ":" ;          // to mark a name definition; usable with find()
constexpr auto JSON_all_delimiters = "{[]}," ;

constexpr unsigned char JSON_SYS_delimiter = '\\' ; // as not allowed in JSON identifiers but printable
constexpr auto          JSON_TOP_Scope = "\\0" ;    // JSON_SYS_delimiter + '0'


using jsonData_type = typename std::string ;

struct s_sectionAtom {     // instantiates a Section described as: "name" = [/{ ..... }/]
                            // _name holds the Full name (all the predecessors), incl. empty names
                                                                // OLD: using iterator = class _iterator ;
    // fields
    std::string     _name{} ;           // the Full name
    long            _addr_start{} ;     // the address of {/[
    long            _addr_end{} ;       // the address of }/]

    s_sectionAtom() : _name{}, _addr_start{-1L}, _addr_end{-1L} {}
    // all special members = default (for now)

    // methods
    bool operator !=(const s_sectionAtom& su) const& {
        return _name != su._name || _addr_start != su._addr_start || _addr_end != su._addr_end ;
    }
    bool section_name_match(const std::string& sec_name) const& {
        size_t  pos ;
        return ((pos = _name.find(sec_name)) != std::string::npos)      	// if _name == sec_name                  
            ? _name.size() == pos + sec_name.size()	// full name match, only
            : false ;
    }
    bool section_name_contains(const std::string& sec_name) const& {
        size_t  pos ;
        return ((pos = _name.find(sec_name)) != std::string::npos)      	// if _name contains sec_name
            ? _name[pos + sec_name.size()] == JSON_SYS_delimiter	// full name match, only
            : false ;
    }
    decltype(auto)  section_level() const& {
        size_t level{0} ;
        for (const auto& el : _name) { if (el == JSON_SYS_delimiter) level++ ; }
        return level ;
    }

    void set_section_name(const std::string& parent, const std::string& n) {
        _name = parent, _name.push_back(JSON_SYS_delimiter), _name += n ;
    }


    friend std::ostream& operator <<(std::ostream& os, const s_sectionAtom& su) {
        os << "(" << su._name << "[" << su._addr_start << ", " << su._addr_end << "])" ;
        return os ;
    }
}; // struct s_sectionAtom


using jsonSections = std::vector<s_sectionAtom> ;

_DEFS_JSON_PARAMETERS_
#endif
// eof json_Iterators.h
