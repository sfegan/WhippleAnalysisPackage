//-*-mode:c++; mode:font-lock;-*-

#ifndef PROGRESSBAR_H
#define PROGRESSBAR_H

#define __STL_USE_NAMESPACES

#include<iostream>
#include<cmath>
#include<string>

#include<time.h>

namespace NS_Analysis {
  
  using std::string;
  using std::ostream;
  
  class ProgressBar
  {
  public:
    ProgressBar(int n): m_n(n), m_i(0) {}
    virtual ~ProgressBar();

    void tick() { bool redraw=must_redraw(m_i+1); m_i++; if(redraw)draw(); }
    void tick(int i) { bool redraw=must_redraw(i); m_i=i; if(redraw)draw(); }
    
    virtual void reset(int n);
    
  protected:
    virtual bool must_redraw(int i) = 0;
    virtual void draw() = 0;

    int m_n;
    int m_i;
  };

  class TextProgressBar: public ProgressBar
  {
  public:
    TextProgressBar(int n=0, ostream& stream=std::cerr, int len=80, 
		    int maxupdateint=3 /* seconds */): 
      ProgressBar(n), m_stream(stream), m_len(len), 
      m_max_update_interval(maxupdateint) {}
    virtual ~TextProgressBar();

    virtual void reset(int n);
    
  private:
    ostream& m_stream;
    int m_len;

    int m_max_update_interval;
    time_t m_start_time; 
    time_t m_update_time;
   
    virtual bool must_redraw(int i);
    virtual void draw();
  };

} // namespace NS_Analysis

#endif // defined PROGRESSBAR_H
