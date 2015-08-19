#include "kstars.h"

class Schedulerjob {
public:
    Schedulerjob();
    enum StateChoice{IDLE, READY, SLEWING, SLEW_COMPLETE, FOCUSING, FOCUSING_COMPLETE, ALIGNING, ALIGNING_COMPLETE, GUIDING, GUIDING_COMPLETE,
                     CAPTURING, CAPTURING_COMPLETE, ABORTED};
    enum SolverChoice{NO_SOLVING, TO_BE_SOLVED, SOLVING, SOLVING_COMPLETE, ERROR};
    QString getName() const;
    void setName(const QString &value);

    QString getRA() const;
    void setRA(const QString &value);

    QString getDEC() const;
    void setDEC(const QString &value);

    QString getStartTime() const;
    void setStartTime(const QString &value);

    QString getFinTime() const;
    void setFinTime(const QString &value);

    QString getFileName() const;
    void setFileName(const QString &value);

    SkyObject *getOb() const;
    void setOb(SkyObject *value);

    float getAlt() const;
    void setAlt(float value);

    float getMoonSeparation() const;
    void setMoonSeparation(float value);

    int getHours() const;
    void setHours(int value);

    int getMinutes() const;
    void setMinutes(int value);

    bool getNowCheck() const;
    void setNowCheck(bool value);

    bool getSpecificTime() const;
    void setSpecificTime(bool value);

    bool getSpecificAlt() const;
    void setSpecificAlt(bool value);

    bool getMoonSeparationCheck() const;
    void setMoonSeparationCheck(bool value);

    bool getMeridianFlip() const;
    void setMeridianFlip(bool value);

    bool getWhenSeqCompCheck() const;
    void setWhenSeqCompCheck(bool value);

    bool getLoopCheck() const;
    void setLoopCheck(bool value);

    bool getOnTimeCheck() const;
    void setOnTimeCheck(bool value);

    int getScore() const;
    void setScore(int value);

    int getFinishingHour() const;
    void setFinishingHour(int value);

    int getFinishingMinute() const;
    void setFinishingMinute(int value);

    bool getFocusCheck() const;
    void setFocusCheck(bool value);

    bool getAlignCheck() const;
    void setAlignCheck(bool value);

    StateChoice getState() const;
    void setState(const StateChoice &value);

    int getRowNumber() const;
    void setRowNumber(int value);

    bool getGuideCheck() const;
    void setGuideCheck(bool value);

    int getIsOk() const;
    void setIsOk(int value);

    bool getParkTelescopeCheck() const;
    void setParkTelescopeCheck(bool value);

    bool getWarmCCDCheck() const;
    void setWarmCCDCheck(bool value);

    bool getCloseDomeCheck() const;
    void setCloseDomeCheck(bool value);

    QString getFITSPath() const;
    void setFITSPath(const QString &value);

    double getFitsRA() const;
    void setFitsRA(double value);

    double getFitsDEC() const;
    void setFitsDEC(double value);

    bool getIsFITSSelected() const;
    void setIsFITSSelected(bool value);

    SolverChoice getSolverState() const;
    void setSolverState(const SolverChoice &value);

    int getMonth() const;
    void setMonth(int value);

    int getDay() const;
    void setDay(int value);

    int getFinishingMonth() const;
    void setFinishingMonth(int value);

    int getFinishingDay() const;
    void setFinishingDay(int value);

private:
    StateChoice state = IDLE;
    SolverChoice solverState = NO_SOLVING;
    QString name;
    QString RA;
    QString DEC;
    QString startTime;
    QString finTime;
    QString fileName;
    QString FITSPath;
    SkyObject *ob;

    float alt;
    float moonSeparation;
    double fitsRA;
    double fitsDEC;
    int hours;
    int minutes;
    int month;
    int day;
    int finishingHour;
    int finishingMinute;
    int finishingMonth;
    int finishingDay;
    int score;
    int isOk;
    int rowNumber;

    bool NowCheck;
    bool specificTime;
    bool specificAlt;
    bool moonSeparationCheck;
    bool meridianFlip;

    bool whenSeqCompCheck;
    bool loopCheck;
    bool onTimeCheck;

    bool focusCheck;
    bool alignCheck;
    bool guideCheck;

    bool parkTelescopeCheck;
    bool warmCCDCheck;
    bool closeDomeCheck;
    bool isFITSSelected;
};