#ifndef Stage_h
#define Stage_h

//#include "Arduino.h"
#include "commands.h"



class Stage
{
  public:
    Stage();
    Stage(cmdType cmd, long temprature, unsigned long duration);
    cmdType cmd();
    long temprature();
    unsigned long duration();

  private:
    cmdType _cmd;
    long _temperature_c;
    unsigned long _duration_sec;
};

#endif
