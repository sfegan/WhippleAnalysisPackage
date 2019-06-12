//-*-mode:c++; mode:font-lock;-*-

#ifndef ODBCPEDSANDGAINSFACTORY_H
#define ODBCPEDSANDGAINSFACTORY_H

#define __STL_USE_NAMESPACES

#include<iostream>
#include<string>
#include<vector>
#include<map>
#include<algorithm>
#include<cmath>

#include<odbc++/drivermanager.h>
#include<odbc++/resultset.h>
#include<odbc++/preparedstatement.h>
#include<odbc++/statement.h>
#include<odbc++/connection.h>
#include<odbc++/types.h>

#include"Types.h"
#include"PedsAndGainsFactory.h"
#include"ChannelData.h"
#include"Pedestals.h"
#include"Gains.h"
#include"UtilityFunctions.h"

namespace NS_Analysis {

  using std::map;
  
  class ODBCPedsAndGainsFactory: public PedsAndGainsFactory
  {
  public:
    virtual void       deletePeds(const string& runname, int date, 
				  const string& compartment="");
    virtual Pedestals*   loadPeds(const string& runname, int date,
				  const string& compartment="");
    virtual void         savePeds(const Pedestals* peds,
				  const string& runname, int date,
				  const string& compartment="");

    virtual void      deleteGains(const string& runname, int date, 
				  const string& compartment="");
    virtual Gains*      loadGains(const string& runname, int date,
				  const string& compartment="");
    virtual void        saveGains(const Gains* gains,
				  const string& runname, int date,
				  const string& compartment="");

    virtual vector<string> listGainsByDate(int date, 
					   const string& compartment="");

    ODBCPedsAndGainsFactory(const string& conn,
			    const string& defcompartment="common");

    virtual ~ODBCPedsAndGainsFactory();

  private:
    odbc::Date makeDate(int date);

    void deleteCVDAM(int file_id,
		     const string& data_table, const string& mask_table);
    void   loadCVDAM(ChannelValDevAndMask* data,
		     int file_id, 
		     const string& data_table, const string& mask_table);
    void   saveCVDAM(const ChannelValDevAndMask* data,
		     int file_id,
		     const string& data_table, const string& mask_table);

    int getCompartmentID(const string& compartment="", bool create=true);

    int getFileID(const string& runname, int date,
		  const string& compartment="", bool create=true);
    int getFileID(const string& runname, const string& date, 
		  const string& compartment="", bool create=true);
    int getFileID(const string& runname, const odbc::Date& date, 
		  const string& compartment="", bool create=true);
    
    odbc::Connection* m_conn;
    void commit() { m_conn->commit(); }

    string m_defaultCompartment;

    odbc::PreparedStatement* prepareStatement(const string& name,
					      const string& sql);
    map<string, odbc::PreparedStatement*> m_statements;
  };   

} // namespace NS_Analysis

#endif // ODBCPEDSANDGAINSFACTORY_H
