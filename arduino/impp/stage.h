#ifndef Stage_h
#define Stage_h


class Stage
{
  public:
    Stage();
    Stage(long temprature, unsigned long duration_sec);

    long temperature();
    unsigned long durationMsec();
    void printStage();
    void resetStage();
    void startStage();
    void finishStage();
    bool tryFinishAndGetIfDone();
    bool isSet();

  private:
    bool _done;
    bool _set;
    long _temperature_c;
    unsigned long _durationMsec;
    unsigned long _startTimeMsec;

};

#endif
