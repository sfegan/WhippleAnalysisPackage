#include"Cleaning.h"

std::vector<unsigned> NS_Analysis::Cleaner::s_count;

void NS_Analysis::Cleaner::resetCamera(CameraConfiguration* cam)
{
  camera=cam;
}

void 
NS_Analysis::CleanerPicBnd::
clean(ChannelData<CleanedState>& clean,
      const ChannelData<double>& signaltonoise) const
{
  const int nchannels=camera->nchannels();
  
  for(int i=0;i<nchannels;i++)
    {
      if(clean(i).disabled())continue;
      if(signaltonoise(i) >= p_level)
	{
	  clean(i).set_state(CleanedState::CL_IMAGEHIGH);
	  for(unsigned int j=0;j<camera->channel(i).numneighbors();j++)
	    {
	      channelnum_type nc=camera->channel(i).neighbor(j);
	      if(clean(nc).disabled())continue;
	      if((!clean(nc).image())&&(signaltonoise(nc) >= b_level))
		clean(nc).set_state(CleanedState::CL_IMAGELOW);
	    }
	}
      else if (!clean(i).image())
	clean(i).set_state(CleanedState::CL_NOTIMAGE);
    }

  if(s_count.size() < nchannels)s_count.resize(nchannels);
  for(int i=0;i<nchannels;i++)if(clean(i).image())s_count[i]++;
}

void
NS_Analysis::CleanerRegional::
clean(ChannelData<CleanedState>& clean,
      const ChannelData<double>& signaltonoise) const
{
  const int nchannels=camera->nchannels();
  int i=0;
  
  // Go through all the tubes and assign them to a "region" of continuous
  // tubes above a threshold of r_level. The "0" region is a pretend one for
  // channels that are below the r_level threshold. Hence the initialisation
  // of region_counts with 1... which skips the "0" entry in the vector
  
  vector<unsigned int> region_counts;
  vector<unsigned int> channel_region(nchannels);
  vector<channelnum_type> channel_stack;

  region_counts.reserve(nchannels/4);
  region_counts.push_back(0); // For tubes that are not part of a region

  channel_stack.reserve(nchannels);

  for(i=0;i<nchannels;i++)
    {
      if(clean(i).disabled())continue;
      unsigned int my_region=0;
      if(!clean(i).unknown())continue; // Already been here
      if(signaltonoise(i) >= r_level)
	{
	  // Its above our lower threshold, start a new region and push this
	  // tube on the stack for later visitation
	  my_region=region_counts.size();
	  region_counts.push_back(0);
	  channel_stack.push_back(i);
	}
      else 
	{
	  clean(i).set_state(CleanedState::CL_NOTIMAGE); 
	  continue;
	}

      while(!channel_stack.empty())
	{
	  channelnum_type ch=channel_stack.back();
	  channel_stack.pop_back();
	  region_counts[my_region]++;
	  channel_region[ch]=my_region;

	  if(signaltonoise(ch) >= i_level)
	    clean(ch).set_state(CleanedState::CL_IMAGEHIGH);
	  else 
	    clean(ch).set_state(CleanedState::CL_NOTIMAGE); // for the moment
	  	  
	  for(unsigned int j=0;j<camera->channel(ch).numneighbors();j++)
	    {
	      channelnum_type nc=camera->channel(ch).neighbor(j);
	      if(clean(nc).disabled())continue;
	      if(clean(nc).unknown())
		{
		  if(signaltonoise(nc) >= r_level)channel_stack.push_back(nc);
		  else clean(nc).set_state(CleanedState::CL_NOTIMAGE); 
		}
	    }
	}
    }
  
  for(i=0;i<nchannels;i++)
    {
      if(!camera->channel(i).isRealChannel())continue;
      if((clean(i).state()==CleanedState::CL_NOTIMAGE)&&
	 (region_counts[channel_region[i]] >= multiplicity))
	clean(i).set_state(CleanedState::CL_IMAGELOW);
    }
}
