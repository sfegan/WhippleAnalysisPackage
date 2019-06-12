class RedHeader {
	string			RunID,
		dim=8,
		comment=Run ID,
		;

	int			NEvents,
		comment=Number of events,
		;

	double			LiveTime,
		comment=Live tme,
		unit=sec,
		;

	int			STDur,
		comment=Siderial duration,
		unit=min,
		;

	string			Mode,
		dim=3,
		comment=Tracking mde,
		;

	string			Source,
		dim=20,
		comment=Source name,
		;

	int			Date,
		comment=Date,
		;

	double			MJD,
		comment=MJD of start of run,
		unit=day,
		;

	double			FRJD,
		comment=who knows,
		;

	float			RA,
		comment=Source RA,
		;

	float 			Dec,
		comment=Source DEC,
		;

	int			UT,
		comment=UT of start of run,
		;

	int			ST,
		comment=Siderial time at run start,
		;

	float			Azimuth,
		comment=Telescope azimuth,
		;

	float			Elevation,
		comment=Telescope elevation,
		;

	string			SkyQ,
		dim=6,
		comment=Sky Quality,
		;

	string			Comms,
		dim=404,
		comment=Comments,
		;

	int 			GPSBeg,
		dim=7,
		comment=Some GPS bits,
		;
}

class RedEvent {
	int			Code,
		comment=Trigger code,
		;
	
	double			Time,
		comment=Event time,
		;

	double			GPSUTC,
		comment=GPS time in UTC,
		;

	double			LiveTime,
		comment=livetime of event,
		;

	short			ADC,
		dim=492,
		include="VersionDims.h",
		dimfunction=NS_Analysis::VersionToADCNumber,
		comment=Raw ADC counts,
		;
}
