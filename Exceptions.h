//-*-mode:c++; mode:font-lock;-*-

#ifndef EXCEPTIONS_Analysis
#define EXCEPTIONS_Analysis

#define __STL_USE_NAMESPACES

#include<string>
#include<iostream>
#include<vector>
#include<sstream>
#include<stdexcept>

#include"Types.h"

namespace NS_Analysis {

  using std::string;
  using std::ostream;
  using std::ostringstream;


  class Error
  {
  public:
    Error(const string& func): m_stream(), m_function(func) {}
    virtual ~Error();

    virtual string exceptionType() const;
    virtual void printLocalErrorStuff(ostream& stream) const;

    const string& function() const { return m_function; }
    const string message() const { return m_stream.str(); }

    ostream& stream() { return m_stream; }

    Error(const Error& e): 
      m_stream(), m_function(e.m_function)
    { m_stream << e.message(); }

  private:
    ostringstream m_stream;
    string m_function;
  };

  class FileError: public Error
  {
  public:
    FileError(const string& func): Error(func) {}

    virtual string exceptionType() const;
  };

  class OutOfRange: public Error
  {
  public:
    OutOfRange(const string& func, int n): Error(func), m_element(n) {}
    int element() const { return m_element; }
    
    virtual string exceptionType() const;

  private:
    int m_element;
  };

  class UnsupportedByVersion : public Error 
  {
  public:
    UnsupportedByVersion(const string& func, const string& what_arg, 
			 unsigned int ver): 
      Error(func), m_arg(what_arg), m_version(ver) { }

    unsigned int version() const { return m_version; };
    const string& argument() const { return m_arg; };

    virtual string exceptionType() const;

  private:
    string m_arg;
    unsigned int m_version;
  };

  class ChannelOutOfRange: public Error
  {
  public:
    ChannelOutOfRange(const string& func, channelnum_type t): 
      Error(func), m_channel(t) {}
    channelnum_type channel() const { return m_channel; }
    
    virtual string exceptionType() const;

  private:
    channelnum_type m_channel;
  };
  
  class ChannelDataIncompatible: public Error
  {
  public:
    ChannelDataIncompatible(const string& func,
			    channelnum_type my, channelnum_type their): 
      Error(func), m_mynchannel(my), m_theirnchannel(their) {}
    
    channelnum_type mynchannel() const { return m_mynchannel; }
    channelnum_type theirnchannel() const { return m_theirnchannel; }
    
    virtual string exceptionType() const;

  private:
    channelnum_type m_mynchannel;
    channelnum_type m_theirnchannel;
  };

  class UnrecognisedParameter: public Error
  {
  public:
    UnrecognisedParameter(const string& func, const string& param):
      Error(func), m_parameter(param) {}
    
    const string& parameter() const { return m_parameter; }

    virtual void printLocalErrorStuff(ostream& stream) const;
    virtual string exceptionType() const;
  
  private:
    string m_parameter;
  };

  class cH5Error: public FileError
  {
  public:
    cH5Error(const string& func): FileError(func) {}
    
    virtual string exceptionType() const;
  };

  /////////////////////////////////////////////////////////////////////////////

  class VSFA_Error: public FileError
  {
  public:
    VSFA_Error(const string& func): FileError(func) {}

    virtual string exceptionType() const;
  };

  class VSFA_ObjectExists: public VSFA_Error
  {
  public:
    VSFA_ObjectExists(const string& func): VSFA_Error(func) {}

    virtual string exceptionType() const;
  };

  class VSFA_RangeError: public VSFA_Error
  {
  public:
    VSFA_RangeError(const string& func, 
		    fasize_type start, fasize_type count, 
		    fasize_type size, fasize_type maxsize): 
      VSFA_Error(func), 
      m_start(start), m_count(count), m_size(size), m_maxsize(maxsize) {}

    virtual string exceptionType() const;
    virtual void printLocalErrorStuff(ostream& stream) const;

    fasize_type start(void) const { return m_start; }
    fasize_type count(void) const { return m_count; }
    fasize_type size(void) const { return m_size; }
    fasize_type maxsize(void) const { return m_maxsize; }

  private:
    fasize_type m_start;
    fasize_type m_count;
    fasize_type m_size;
    fasize_type m_maxsize;
  };

  ostream& operator<<(ostream& stream, const Error &x);
  
} // namespace NS_Analysis

#endif // EXCEPTIONS_Analysis
