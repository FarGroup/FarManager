#ifndef __BASEINPUT_HPP__
#define __BASEINPUT_HPP__
/*
baseinp.hpp

Базовый класс (абстрактный класс)

*/

class BaseInput
{
  public:
    virtual int ProcessKey(int Key) { return(0); };
    virtual int ProcessMouse(MOUSE_EVENT_RECORD *MouseEvent) { return(0); };
};


#endif  // __BASEINPUT_HPP__
