#pragma once

#include "BaseType.h"

enum ENUM_APP_MODE
{
    APP_MODE_INVALID,  //��Ч 
    APP_MODE_PLAN,  //�ƻ�
    APP_MODE_EVENTS,  //�Զ����� 
    APP_MODE_TODO,  //todo 
    APP_MODE_DING,  //��� ding~~ ����
};

namespace littlet
{
    void EnterAppMode(ENUM_APP_MODE eMode);
    ENUM_APP_MODE GetAppMode();

    /** ctrl�����£��п����Ǳ��ؿ�ݼ������ô˺������������Ӧ����
     *	return
     *      SetMsgHandled(OnCtrlKeyPressDown(...));
     *	param
     *		-[in]
     *          nChar       vk_xx
    **/
    BOOL OnCtrlKeyPressDown(UINT nChar,UINT nRepCnt,UINT nFlags);
};

class LViewEvent;
class LViewTodo;
class LViewPlan;

class LDatas
{
    friend void littlet::EnterAppMode(ENUM_APP_MODE eMode);
     
    SINGLETON_ON_DESTRUCTOR(LDatas)
    {

    }
public:
    void SetViewPlanPtr(LViewPlan *p)
    {
        m_pViewPlan = p;
    }

    LViewPlan* GetViewPlanPtr()
    {
        return m_pViewPlan;
    }

    void SetViewEventPtr(LViewEvent* p )
    {
        m_pViewEvent = p;
    }

    LViewEvent* GetViewEventPtr()
    {
        return m_pViewEvent;
    }

    void SetViewTodoPtr(LViewTodo *p)
    {
        m_pViewTodo = p;
    }

    LViewTodo* GetViewTodoPtr()
    {
        return m_pViewTodo;
    }

    ENUM_APP_MODE GetAppMode()const
    {
        return m_eMode;
    }

protected:
    void SetAppMode(ENUM_APP_MODE eMode)
    {
        m_eMode = eMode;
    }

private:
    ENUM_APP_MODE       m_eMode;
    LViewEvent*         m_pViewEvent;
    LViewTodo*          m_pViewTodo;
    LViewPlan*          m_pViewPlan;
};

