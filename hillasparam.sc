class HillasParam {
	double			Max1,
		comment=value of max tube after gains,
		unit=dc,
		cuts=lower,
		;

	double			Max2,
		comment=value of 2nd to max tube after gains,
		unit=dc,
		cuts=lower,
		;

	double			Max3,
		comment=value of 3rd to max tube after gains,
		unit=dc,
		cuts=lower,
		;

	unsigned~int		Loc1,
		comment=tube number of max tube,
		unit=tubeno,
		;


	unsigned~int		Loc2,
		comment=tube number of 2nd to max tube,
		unit=tubeno,
		;

	unsigned~int		Loc3,
		comment=tube number of 3rd to max tube,
		unit=tubeno,
		;

	unsigned~int		NImage,
		comment=number of image tubes,
		unit=tubeno,
		cuts=both,
		;

	double 			Size,
		comment=total amount of light in image,
		unit=dc,
		cuts=both,
		;

	double			X0,
		comment=x-coordinate of origin,
		unit=deg,
		;

	double			Y0,
		comment=y-coordinate of origin,
		unit=deg,
		;

	double			XC,
		comment=x-coordinate of centroid,
		unit=deg,
		;

	double			YC,
		comment=y-coordinate of centroid,
		unit=deg,
		;

	double			Dist,
		comment=distance to centroid,
		unit=deg,
		cuts=both,
		;

	double			Length,
		comment=size of semi-major axis,
		unit=deg,
		cuts=both,
		;

	double			Width,
		comment=size of semi-minor axis,
		unit=deg,
		cuts=both,
		;

	double			Miss,
		comment=dist from origin to semi-major axis,
		unit=deg,
		cuts=both,
		;

	double			CosPsi,
		comment=directional cosine of semi-major axis,
		unit=none,
		;

	
	double			SinPsi,
		comment=directional sine of semi-major axis,
		unit=none,
		;

	double			Azwidth,
		comment=how to describe this... its azwidth,
		unit=deg,
		;

	double			Asymmetry,
		comment=asymmetry along semi-major axis,
		unit=none,
		cuts=both,
		;

	double			MinorAsymmetry,
		comment=asymmetry along semi-minor axis,
		unit=none,
		cuts=both;
		;

	double			Alpha,
		comment=ANGLE [semi-major axis]^[origin->centroid],
		unit=deg,
		cuts=both,
		;

	double			SinAlpha,
		comment=sine of alpha,
		unit=none,
		;

	double			LengthOverSize,
		comment=length over size,
		unit=deg/dc,
		cuts=both,
		;
};

class EventInfo {
	int			EventNumber,
		comment=Sequential event number in raw data file,
		;

	int			Code,
		comment=Trigger code,
		;
	
	double			Time,
		comment=Event time,
		cuts=both,
		;

	double			GPSUTC,
		comment=GPS time in UTC,
		cuts=both,
		;

	double			LiveTime,
		comment=livetime of event,
		cuts=both,
		;
};
