#pragma once

/*
 *	 ����ѹ�����ԵĴ���
 */

#include <QThread>

class KPressureTest : public QThread
{
public:
    KPressureTest();
    ~KPressureTest();

protected:
    virtual void run();

};

