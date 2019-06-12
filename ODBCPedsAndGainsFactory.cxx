#include"Pedestals.h"
#include"PedsAndGainsFactory.h"
#include"ODBCPedsAndGainsFactory.h"
#include"Exceptions.h"

using std::map;
using std::vector;
using std::string;

NS_Analysis::ODBCPedsAndGainsFactory::
ODBCPedsAndGainsFactory(const string& conn, 
			const string& defcompartment):
  m_defaultCompartment(defcompartment)
{
  m_conn = odbc::DriverManager::getConnection(conn);
  m_conn->setAutoCommit(false);
}

NS_Analysis::ODBCPedsAndGainsFactory::
~ODBCPedsAndGainsFactory()
{
  map<string, odbc::PreparedStatement*>::iterator i = m_statements.begin();
  while(i != m_statements.end())
    {
      //      cerr << "delete: " << i->first << '=' << i->second << endl;
      delete i->second;
      i++;
    }

  delete m_conn;
}

NS_Analysis::Pedestals* 
NS_Analysis::ODBCPedsAndGainsFactory::
loadPeds(const string& runname, int date, const string& compartment)
{
  int id = getFileID(runname,date,compartment,false);
  if(id == 0)return 0;

  odbc::ResultSet* rs=0;

  ///////////////////////// First : get pedestal info /////////////////////////

  odbc::PreparedStatement* ps_lockpeds=
    prepareStatement("lock_pedestals",
		     "LOCK TABLE pedestals");

  odbc::PreparedStatement* ps_selpedsbyid=
    prepareStatement("select_peds_by_id",
		     "SELECT * FROM pedestals WHERE ( file_id=? )");

  ps_lockpeds->execute();

  ps_selpedsbyid->setInt(1,id);
  rs=ps_selpedsbyid->executeQuery();
  if(!rs->next())
    {
      delete rs;
      commit();
      return 0;
    }

  Pedestals* peds=new Pedestals(rs->getInt("nchannels"));
  peds->setNEvents(rs->getInt("nevents"));
  peds->setCamera(rs->getString("camera_name"));
  peds->setComment(rs->getString("comment"));
  
  if(rs->next())
    {
      delete peds;
      commit();
      // ERROR -- more than one returned by DB
      throw Error("ODBCPedsAndGainsFactory::loadPeds 1");
    }

  delete rs;

  loadCVDAM(peds,id,"peds_data","peds_mask_data");

  commit();
  return peds;
}

void
NS_Analysis::ODBCPedsAndGainsFactory::
savePeds(const Pedestals* peds,
	 const string& runname, int date, const string& compartment)
{
  int id = getFileID(runname,date,compartment);

  odbc::PreparedStatement* ps_lockpeds=
    prepareStatement("lock_pedestals",
		     "LOCK TABLE pedestals");

  odbc::PreparedStatement* ps_insped=
    prepareStatement("insert_peds",
		     "INSERT INTO pedestals VALUES ( ?, ?, ?, ?, ? )");

  ps_lockpeds->execute();
  
  ps_insped->setInt(1,id);
  ps_insped->setString(2,peds->camera());
  ps_insped->setInt(3,peds->nchannels());
  ps_insped->setInt(4,peds->nevents());
  ps_insped->setString(5,peds->comment());
  if(ps_insped->executeUpdate() != 1)
    {
      // ERROR
      commit();
      throw Error("ODBCPedsAndGainsFactory::setPeds 1");
    }

  saveCVDAM(peds,id,"peds_data","peds_mask_data");

  commit();
}

void
NS_Analysis::ODBCPedsAndGainsFactory::
deletePeds(const string& runname, int date, const string& compartment)
{
  int id = getFileID(runname,date,compartment);

  odbc::PreparedStatement* ps_lockpeds=
    prepareStatement("lock_pedestals",
		     "LOCK TABLE pedestals");

  odbc::PreparedStatement* ps_delpedbyid=
    prepareStatement("delete_peds_by_id",
		     "DELETE FROM pedestals WHERE ( file_id=? )");

  ps_lockpeds->execute();

  ps_delpedbyid->setInt(1,id);
  ps_delpedbyid->executeUpdate();

  deleteCVDAM(id,"peds_data","peds_mask_data");

  commit();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

NS_Analysis::Gains* 
NS_Analysis::ODBCPedsAndGainsFactory::
loadGains(const string& runname, int date, const string& compartment)
{
  int id = getFileID(runname,date,compartment,false);
  if(id == 0)return 0;

  odbc::ResultSet* rs=0;

  ///////////////////////// First : get gain info /////////////////////////

  odbc::PreparedStatement* ps_lockgains=
    prepareStatement("lock_gains",
		     "LOCK TABLE gains");

  odbc::PreparedStatement* ps_selgainsbyid=
    prepareStatement("select_gains_by_id",
		     "SELECT * FROM gains WHERE ( file_id=? )");

  ps_lockgains->execute();

  ps_selgainsbyid->setInt(1,id);
  rs=ps_selgainsbyid->executeQuery();
  if(!rs->next())
    {
      delete rs;
      commit();
      return 0;
    }

  Gains* gains=new Gains(rs->getInt("nchannels"));
  gains->setNEvents(rs->getInt("nevents"));
  gains->setCamera(rs->getString("camera_name"));
  gains->setComment(rs->getString("comment"));
  gains->setMeanSignalMean(rs->getDouble("mean_signal_mean"));
  gains->setMeanSignalDev(rs->getDouble("mean_signal_dev"));
  
  if(rs->next())
    {
      delete gains;
      commit();
      // ERROR -- more than one returned by DB
      throw Error("ODBCPedsAndGainsFactory::loadGains 1");
    }

  delete rs;

  loadCVDAM(gains,id,"gains_data","gains_mask_data");

  commit();
  return gains;
}

void
NS_Analysis::ODBCPedsAndGainsFactory::
saveGains(const Gains* gains,
	 const string& runname, int date, const string& compartment)
{
  int id = getFileID(runname,date,compartment);

  odbc::PreparedStatement* ps_lockgains=
    prepareStatement("lock_gains",
		     "LOCK TABLE gains");

  odbc::PreparedStatement* ps_insped=
    prepareStatement("insert_gains",
		     "INSERT INTO gains VALUES ( ?, ?, ?, ?, ?, ?, ? )");

  ps_lockgains->execute();
  
  ps_insped->setInt(1,id);
  ps_insped->setString(2,gains->camera());
  ps_insped->setInt(3,gains->nchannels());
  ps_insped->setInt(4,gains->nevents());
  ps_insped->setString(5,gains->comment());
  ps_insped->setDouble(6,gains->meanSignalMean());
  ps_insped->setDouble(7,gains->meanSignalDev());
  if(ps_insped->executeUpdate() != 1)
    {
      // ERROR
      commit();
      throw Error("ODBCPedsAndGainsFactory::setGains 1");
    }

  saveCVDAM(gains,id,"gains_data","gains_mask_data");

  commit();
}

void
NS_Analysis::ODBCPedsAndGainsFactory::
deleteGains(const string& runname, int date, const string& compartment)
{
  int id = getFileID(runname,date,compartment);

  odbc::PreparedStatement* ps_lockgains=
    prepareStatement("lock_gains",
		     "LOCK TABLE gains");

  odbc::PreparedStatement* ps_delpedbyid=
    prepareStatement("delete_gains_by_id",
		     "DELETE FROM gains WHERE ( file_id=? )");

  ps_lockgains->execute();

  ps_delpedbyid->setInt(1,id);
  ps_delpedbyid->executeUpdate();

  deleteCVDAM(id,"gains_data","gains_mask_data");

  commit();
}

vector<string> 
NS_Analysis::ODBCPedsAndGainsFactory::
listGainsByDate(int date, const string& compartment)
{
  int compartment_id;
  compartment_id = getCompartmentID(compartment,false);
  if(compartment_id == 0)
    {
      commit();
      return vector<string>(0);
    }

  odbc::PreparedStatement* ps_lockgains=
    prepareStatement("lock_gains",
		     "LOCK TABLE gains");

  odbc::PreparedStatement* ps_listgains=
    prepareStatement("list_gains_by_date",
		     "SELECT name FROM data_file df, gains g WHERE "
		     "( df.file_id=g.file_id AND utc_date=? AND "
		     "compartment_id=? )");
  
  ps_lockgains->execute();

  ps_listgains->setDate(1,makeDate(date));
  ps_listgains->setInt(2,compartment_id);

  odbc::ResultSet* rs=0;
  rs=ps_listgains->executeQuery();

  vector<string> names;
  while(rs->next())
    {
      names.push_back(rs->getString("name"));
    }

  delete rs;
  commit();

  return names;
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// PRIVATE STUFF ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


void 
NS_Analysis::ODBCPedsAndGainsFactory::
deleteCVDAM(int file_id, const string& data_table, const string& mask_table)
{
  odbc::PreparedStatement* ps_deldatabyid=
    prepareStatement("delete_"+data_table+"_by_id",
		     "DELETE FROM "+data_table+" WHERE ( file_id=? )");
  
  odbc::PreparedStatement* ps_delmaskbyid=
    prepareStatement("delete_"+mask_table+"_by_id",
		     "DELETE FROM "+mask_table+" WHERE ( file_id=? )");
  
  ps_deldatabyid->setInt(1,file_id);
  ps_deldatabyid->executeUpdate();

  ps_delmaskbyid->setInt(1,file_id);
  ps_delmaskbyid->executeUpdate();
}

void
NS_Analysis::ODBCPedsAndGainsFactory::
loadCVDAM(ChannelValDevAndMask* data,
	  int file_id, 
	  const string& data_table, const string& mask_table)
{
  odbc::ResultSet* rs=0;

  ////////////////////////////////// 1: DATA //////////////////////////////////

  odbc::PreparedStatement* ps_seldatabyid=
    prepareStatement("select_"+data_table+"_by_id",
		     "SELECT * FROM "+data_table+" WHERE ( file_id=? )");
  
  ps_seldatabyid->setInt(1,file_id);
  rs=ps_seldatabyid->executeQuery();
  
  while((rs)&&(rs->next()))
    {
      int c=rs->getInt(2);
      data->setVal(c,rs->getDouble(3));
      data->setDev(c,rs->getDouble(4));
    }
  
  delete rs;

  ////////////////////////////////// 2: MASK //////////////////////////////////

  odbc::PreparedStatement* ps_selmaskbyid=
    prepareStatement("select_"+mask_table+"_by_id",
		     "SELECT * FROM "+mask_table+" WHERE ( file_id=? )");
  
  ps_selmaskbyid->setInt(1,file_id);
  rs=ps_selmaskbyid->executeQuery();

  while(rs->next())
    {
      int c=rs->getInt(2);
      data->mask(c).mask(rs->getString(3));
    }
  
  delete rs;
}

void   
NS_Analysis::ODBCPedsAndGainsFactory::
saveCVDAM(const ChannelValDevAndMask* data,
	  int file_id,
	  const string& data_table, const string& mask_table)
{
  odbc::PreparedStatement* ps_insdata=
    prepareStatement("insert_"+data_table+"_by_id",
		     "INSERT INTO "+data_table+" VALUES ( ?, ?, ?, ? )");

  odbc::PreparedStatement* ps_insmask=
    prepareStatement("insert_"+mask_table+"_by_id",
		     "INSERT INTO "+mask_table+" VALUES ( ?, ?, ? )");
  
  for(unsigned int c=0; c<data->nchannels(); c++)
    {
      //////////////////////////////// 1: DATA ////////////////////////////////
      ps_insdata->setInt(1,file_id);
      ps_insdata->setInt(2,c);
      ps_insdata->setDouble(3,data->val(c));
      ps_insdata->setDouble(4,data->dev(c));
      if(ps_insdata->executeUpdate() != 1)
	{
	  // ERROR
	  commit();
	  throw Error("ODBCPedsAndGainsFactory::saveCVDAM 1");
	}

      //////////////////////////////// 2: MASK ////////////////////////////////
      if(data->mask(c).isMasked())
	{
	  int nreasons=data->mask(c).nMaskedReasons();
	  for(int r=0; r<nreasons; r++)
	    {
	      ps_insmask->setInt(1,file_id);
	      ps_insmask->setInt(2,c);
	      ps_insmask->setString(3,data->mask(c).maskedReason(r));
	      if(ps_insmask->executeUpdate() != 1)
		{
		  // ERROR
		  commit();
		  throw Error("ODBCPedsAndGainsFactory::setPeds 3");
		}
	    }
	}
    }
}

odbc::PreparedStatement* 
NS_Analysis::ODBCPedsAndGainsFactory::
prepareStatement(const string& name, const string& sql)
{
  odbc::PreparedStatement*& ps=m_statements[name];
  if(ps==0)
    {
      //      cerr << "prep: " << name << "=" << sql << endl;
      ps=m_conn->prepareStatement(sql);
    }
  return ps;
}

int 
NS_Analysis::ODBCPedsAndGainsFactory::
getCompartmentID(const string& compartment, bool create)
{
  string name=compartment;
  if(name.size() == 0)name=m_defaultCompartment;

  odbc::PreparedStatement* ps_lock=
    prepareStatement("lock_compartments",
		     "LOCK TABLE compartments");
  
  odbc::PreparedStatement* ps_selid=
    prepareStatement("select_compartment_id",
		     "SELECT compartment_id FROM compartments"
		     " WHERE ( name=? )");
  
  ps_lock->execute();

  ps_selid->setString(1,name);
  odbc::ResultSet* rs = ps_selid->executeQuery();
  if(rs->next()) // The ID is already in the database so return it
    {
      int id=rs->getInt(1);
      delete rs;
      commit();
      return id;
    }
  delete rs;

  if(create == false)
    {
      commit();
      return 0;
    }

  // ELSE: make a new place for it

  odbc::PreparedStatement* ps_insfile=
    prepareStatement("insert_compartments",
		     "INSERT INTO compartments VALUES"
		     " ( ?, NEXTVAL('compartment_id_seq') )");
  
  ps_insfile->setString(1,name);
  if(ps_insfile->executeUpdate() != 1)
    {
      // ERROR
      throw Error("ODBCPedsAndGainsFactory::getCompartmentID");
    }
  
  ps_selid->setString(1,name);
  rs = ps_selid->executeQuery();
  if(rs->next()) // The ID is already in the database so return it
    {
      int id=rs->getInt(1);
      delete rs;
      commit();
      return id;
    }
  delete rs;

  // ERROR
  commit();
  throw Error("ODBCPedsAndGainsFactory::getCompartmentID");
  return 0;
}

odbc::Date 
NS_Analysis::ODBCPedsAndGainsFactory::
makeDate(int date)
{
  fixDate2000(date);
  odbc::Date D;
  D.setDay(date%100);
  D.setMonth(date/100 % 100);
  D.setYear(date/10000);
  return D;
}

int 
NS_Analysis::ODBCPedsAndGainsFactory::
getFileID(const string& runname, int date, 
	  const string& compartment, bool create)
{
  return getFileID(runname,makeDate(date),compartment,create);
}

int 
NS_Analysis::ODBCPedsAndGainsFactory::
getFileID(const string& runname, const string& date, 
	  const string& compartment, bool create)
{
  return getFileID(runname,odbc::Date(date),compartment,create);
}

int 
NS_Analysis::ODBCPedsAndGainsFactory::
getFileID(const string& runname, const odbc::Date& date,
	  const string& compartment, bool create)
{
  int compartment_id;
  compartment_id = getCompartmentID(compartment,create);
  if(compartment_id == 0)
    {
      return 0;
    }

  odbc::PreparedStatement* ps_lock=
    prepareStatement("lock_data_file",
		     "LOCK TABLE data_file");
  
  odbc::PreparedStatement* ps_selid=
    prepareStatement("select_file_id",
		     "SELECT file_id FROM data_file WHERE"
		     " ( name=? AND utc_date=? AND compartment_id=? )");
  
  ps_lock->execute();

  ps_selid->setString(1,runname);
  ps_selid->setDate(2,date);
  ps_selid->setInt(3,compartment_id);
  odbc::ResultSet* rs = ps_selid->executeQuery();
  if(rs->next()) // The ID is already in the database so return it
    {
      int id=rs->getInt(1);
      delete rs;
      commit();
      return id;
    }
  delete rs;

  if(create == false)
    {
      commit();
      return 0;
    }

  // ELSE: make a new place for it

  odbc::PreparedStatement* ps_insfile=
    prepareStatement("insert_data_file",
		     "INSERT INTO data_file VALUES"
		     " ( ?, ?, ?, NEXTVAL('file_id_seq') )");

  ps_insfile->setString(1,runname);
  ps_insfile->setDate(2,date);
  ps_insfile->setInt(3,compartment_id);
  if(ps_insfile->executeUpdate() != 1)
    {
      // ERROR
      throw Error("ODBCPedsAndGainsFactory::getFileID");
    }
  
  ps_selid->setString(1,runname);
  ps_selid->setDate(2,date);
  ps_selid->setInt(3,compartment_id);
  rs = ps_selid->executeQuery();
  if(rs->next()) // The ID is already in the database so return it
    {
      int id=rs->getInt(1);
      delete rs;
      commit();
      return id;
    }
  delete rs;

  // ERROR
  commit();
  throw Error("ODBCPedsAndGainsFactory::getFileID");
  return 0;
}
