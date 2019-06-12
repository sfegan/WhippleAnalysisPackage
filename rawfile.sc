class TelescopeEvent {
	H5T_STD_U32LE		event,
		comment= event number - possibly non-sequential ,
		;

	H5T_IEEE_F64LE	        ^utc,
		comment=GPS mean time of event             [mjd],
		flc=Event Time Information,
		;

	H5T_UNIX_D32LE		time_sec,
		comment=GPS event time (UNIX epoch)        [sec],
		dim=NGPS,	
		;

	H5T_STD_U32LE		time_ns,
		comment=GPS event nanosecond offset         [ns],
		dim=NGPS,
		;

	H5T_STD_U32LE		live_sec,
		comment=live time from start of run        [sec],
		;

	H5T_STD_U32LE		live_^ns,
		comment=live time from last sec             [ns],
		;

	H5T_STD_U32LE		elapsed_sec,
		comment=sec from start of run              [sec],
		;
	H5T_STD_U32LE		elapsed_^ns,
		comment=ns since last elapsed sec           [ns],
		;


	H5T_STD_B8LE		trigger,
		comment=trigger information             [8 bits],
		flc=Trigger information,
		;

	H5T_STD_B32LE		cfd_pattern, 
		comment=CFD fire pattern               [32 bits],
		dim=NCFDW32,
		;

	H5T_STD_U32LE		^adc_readout_store_start,
		comment=Start of ADC values for this event      ,
		flc=ADC value information,
		interface=no,
		;

	H5T_STD_U16LE		^adc_readout_count,
		comment=Number of ADC values in this event      ,
		interface=no,
		;

	H5T_STD_U32LE		track,
		comment=telescope angle encoders          [bits],
		flc=Encoder Information,
		dim=2,
		;
};

class ADCvalue {
	H5T_STD_U16LE		^pmt,
		comment=PMT channel number for this sample      ,

	H5T_STD_U16LE		charge,
		comment=integrated charge over gate             ,
		;

	H5T_STD_B8LE		status,
		comment=various status bits for the value       ,
		;

	H5T_STD_U16LE		pedestal,
		comment=pedestal value                          ,
		;

	H5T_STD_U8LE		profile, 
		dim=16,
		comment=FADC time profile                       ,
		;
};



class PSTPatchCount {
	H5T_UNIX_D32LE		time,
		comment=time of patch count (UNIX epoch)   [sec],
		;

	H5T_STD_U32LE		patch_count,
		comment=PST patch fire scalar counts            ,
		dim=NPSTP,
		;
};

class VoltageAdjustment {
	H5T_UNIX_D32LE		time,
		comment=time of adjustment (UNIX epoch)    [sec],
		;

	H5T_STD_U16LE		channel,
		comment=channel number                          ,
		;

	H5T_STD_I16LE		voltage
		comment=voltage for tube			,
		;
}

class HVSupplyReading {
	H5T_UNIX_D32LE		time,
		comment=time of adjustment (UNIX epoch)    [sec],
		;


	H5T_STD_I16LE		demand_voltage
		comment=requested voltage for tube	     [V],
		ndim=NCHAN,
		;

	H5T_STD_I16LE		measured_voltage
		comment=measured voltage for tube	     [V],
		ndim=NCHAN,
		;

	H5T_STD_I16LE		supply_current
		comment=current being drawn by tube	    [uA],
		ndim=NCHAN,
		;
}

class CCDImage {
	H5T_UNIX_D32LE		time,
		comment=time of ccd image (UNIX epoch)     [sec],
		;

	H5T_STD_U16LE		cross_x_pixel,
		comment=x coordinate of crosshairs       [pixel],
		;

	H5T_STD_U16LE		cross_y_pixel,
		comment=y coordinate of crosshairs       [pixel],
		;

	H5T_STD_U16LE		average_bright,
		comment=average image brightness            [dc],
		;

	H5T_STD_U16LE		star_x_pixel,
		comment=x coordinate of star in ccd      [pixel],
		dim=CCDSTARS,
		;

	H5T_STD_U16LE		star_y_pixel,
		comment=y coordinate of star in ccd      [pixel],
		dim=CCDSTARS,
		;

	H5T_STD_U16LE		star_bright_dc,
		comment=brightness in digital counts        [dc],
		dim=CCDSTARS,
		;
}

class SourceInfo {
	H5T_IEEE_F64LE		ra,
		comment=right assention of source          [deg],
		;

	H5T_IEEE_F64LE		dec,
		comment=declination of source              [deg],
		;
}

class Tracking {
	H5T_UNIX_D32LE		time,
		comment=time of tracking info (UNIX epoch) [sec],
		;

	H5T_IEEE_F64LE		elevation,
		comment=elevation of telescope             [deg],
		;

	H5T_IEEE_F64LE		azimuth,  
		comment=azimuthal angle of telescope       [deg],
		;

	H5T_STD_B8LE		status,
		comment=mount and tracking status         [bits],
		;
}

class CurrentMonitor {
	H5T_UNIX_D32LE		time,
		comment=time of current reading (UNIX ep)  [sec],
		;

	H5T_STD_I16LE		current,
		comment=anode current in pmt                [uA],
		dim=NPMT,
		;
}
