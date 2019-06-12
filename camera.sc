class StorableChannelMask {
        unsigned~int		Num,
		comment=channel number,
		;

        string                  Reason,
                dim=21,
		comment=why is this tube masked,
                ;
}

class StorableChannelDescription {
	hbool_t			RealChannel,
		comment=is there a real tube in this channel,
		;

	unsigned~int		Num,
		comment=channel number,
		;

	double			X,
		comment=the x-coordinate in the camera plane,
		unit=deg,
		;

	double			Y,
		comment=the y-coordinate in the camera plane,
		unit=deg,
		;

	double			R,
		comment=the radius of the tube,
		unit=deg,
		;

	unsigned~int		Population,
		comment=population this tube is a member of,
		;

	double			RelativeCollectionArea,
		comment=relative collection efficiency of tube,
		;

	string			PrintableName,
		dim=11,
		comment=human readable label of this channel,
		;

	hbool_t			Masked,
		comment=tube is masked off,
		;
}

class StorableNeighborVertex {
	unsigned~int		ChannelA,
		comment=one channel making up the vertex,
		;

	unsigned~int		ChannelB,
		comment=the other channel on the vertex,
		;
}

class StorableCameraConfiguration {
        string                  Description
                dim=41,
                comment=short textual description of camera
                ;

	unsigned~int		NChannel,
		comment=number of channels in camera
		;

	unsigned~int		NTrigger,
		comment=number of channels in the trigger
		;

	double			Latitude,
		comment=latitude of observatory site,
		unit=deg,
		;

	double			Longitude,
		comment=longitude of observatory site,
		unit=deg,
		;

	double			Elevation,
		comment=elevation of observatory site,
		unit=m,
		;
}
