#ifndef _ERRORS_H
#define _ERRORS_H

#include <iostream>
#include <string>
#include <utility>

class argument_error {
    std::string errmsg;
public:
    friend std::ostream& operator << (std::ostream& os, const argument_error & ae) {
        os << "Argument error: " << ae.errmsg;
        return os;
    }
    
    explicit argument_error(const char * str) : errmsg(str) {}
};

class file_open_error {
    std::string filename;
public:
    friend std::ostream& operator << (std::ostream& os, const file_open_error & fe) {
        os << "Error opening " << fe.filename;
        return os;
    }

    explicit file_open_error(std::string name) : filename(std::move(name)) {}
};

class parse_error {
public:
    virtual void print_self(std::ostream& os) const {
        os << "unspecified.";
    }

    friend std::ostream& operator << (std::ostream& os, const parse_error & pe) {
        os << "Parse error: ";
        pe.print_self(os);
        return os;
    }
    virtual ~parse_error() {}
};

class parse_error_str : public parse_error {
    std::string str;
public:
    virtual void print_self(std::ostream& os) const {
        os << str;
    }

    explicit parse_error_str(std::string s) : str(std::move(s)) {}
};

class size_mismatch : public parse_error {
    const unsigned long real_size, read_size;
public:
    virtual void print_self(std::ostream& os) const {
        os << "expected " << real_size << " bits, read " << read_size;
    }

    size_mismatch(unsigned long real_s, unsigned long read_s) : real_size(real_s), read_size(read_s) {}
};

class invalid_id : public parse_error {
    const int id;
public:
    virtual void print_self(std::ostream& os) const {
        os << "invalid codebook id " << id << ", try --inline-codebooks";
    }

    explicit invalid_id(int i) : id(i) {}
};


#endif
