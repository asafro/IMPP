#ifndef Stages_manager_h
#define Stages_manager_h

//#include "Arduino.h"
#include "stage.h"

const unsigned int MAX_STAGES = 1000;

class StagesManager
{
  public:
    StagesManager();
    void addStage(Stage s);
    bool hasMoreStages();
    Stage& getNextStage();
    
  private:
    Stage _stages[MAX_STAGES];
    unsigned int _cur_stage = 0;
    unsigned int _num_stages = 0;
};

#endif
