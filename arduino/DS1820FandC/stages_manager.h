#ifndef Stages_manager_h
#define Stages_manager_h

#include "stage.h"

const unsigned int MAX_STAGES = 1000;

class StagesManager
{
  public:
    StagesManager();
    void addStage(Stage s);
    void addStage(int temperatureC, int timeMs);
    void resetStages();
    void printStages();
    Stage* getStage();
  private:
    Stage _stages[MAX_STAGES];
    unsigned int _curStage = -1;
    unsigned int _numStages = 0;
};

#endif
