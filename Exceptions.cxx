#include<iostream>
#include<string>

#include"Exceptions.h"

using std::ostream;
using std::string;
using std::endl;

NS_Analysis::Error::
~Error()
{
}

string 
NS_Analysis::Error::
exceptionType() const
{
  return "Error";
}

void 
NS_Analysis::Error::
printLocalErrorStuff(ostream& stream) const
{
}

string 
NS_Analysis::FileError::
exceptionType() const
{
  return "FileError";
}

string 
NS_Analysis::OutOfRange::
exceptionType() const
{
  return "OutOfRange";
}

string 
NS_Analysis::UnsupportedByVersion::
exceptionType() const
{
  return "UnsupportedByVersion";
}

string 
NS_Analysis::ChannelOutOfRange::
exceptionType() const
{
  return "ChannelOutOfRange";
}

string 
NS_Analysis::ChannelDataIncompatible::
exceptionType() const
{
  return "ChannelDataIncompatible";
}

void 
NS_Analysis::UnrecognisedParameter::
printLocalErrorStuff(ostream& stream) const
{
  stream << "Unrecognised parameter: " << parameter() << endl;
}
 
string 
NS_Analysis::UnrecognisedParameter::
exceptionType() const
{
  return "UnrecognisedParameter";
}

string 
NS_Analysis::cH5Error::
exceptionType() const
{
  return "cH5Error";
}

string 
NS_Analysis::VSFA_Error::
exceptionType() const
{
  return "VSFA_Error";
}

string 
NS_Analysis::VSFA_ObjectExists::
exceptionType() const
{
  return "VSFA_ObjectExists";
}

string 
NS_Analysis::VSFA_RangeError::
exceptionType() const
{
  return "VSFA_RangeError";
}

void 
NS_Analysis::VSFA_RangeError::
printLocalErrorStuff(ostream& stream) const
{
  stream << "Range: " << start() << '+' << count() << ' '
	 << "size: " << size() << '/' << maxsize() << '\n';
}

ostream& 
NS_Analysis::operator<<(ostream& stream, const NS_Analysis::Error &x) 
{
  stream << "Exception " << x.exceptionType() << " thrown in function " 
	 << x.function() << endl;
  x.printLocalErrorStuff(stream);
  stream << "Error Message:" << endl << x.message() << endl;
  return stream;
}
