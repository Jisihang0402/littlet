﻿#include "QDBHelper.h"

#define DEFAULT_TODOTASK_PRIORITY 3

BOOL QDBEvents::Startup()
{
    return Open(L"helper.db");
}

void QDBEvents::OnDestruct()
{
}

BOOL QDBEvents::TodoTask_GetUnfinished(TodoTaskList &lst)
{
    CStdStringW sQ;
    sQ.Format(L"SELECT * FROM tbl_todo WHERE (Status = %d)", TODO_STATUS_PROCESSING);
    return TodoTask_Read(sQ, TODOTASK_TYPE_TODO, lst);
}

BOOL QDBEvents::TodoTask_GetFinished(TodoTaskList &lst)
{
    CStdStringW sQ;
    sQ.Format(L"SELECT * FROM tbl_todo WHERE (Status = %d)", TODO_STATUS_FINISH);
    return TodoTask_Read(sQ, TODOTASK_TYPE_TODO, lst);
}

BOOL QDBEvents::TodoTask_GetAll(TodoTaskList &lst)
{
    return TodoTask_Read(L"SELECT * FROM tbl_todo", TODOTASK_TYPE_TODO, lst);
}

BOOL QDBEvents::TodoTask_GetAllStickyNote(TodoTaskList &lst)
{
    CStdString sql;
    sql.Format(L"SELECT * FROM tbl_todo WHERE Flag<>0 AND Status=%d", TODO_STATUS_PROCESSING);
    return TodoTask_Read(sql, TODOTASK_TYPE_STICKNOTE, lst);
}

// BOOL QHelperMan::TodoTask_Get(BOOL bActiveAndDone,ENUM_TASK_STATUS eStatus,
// 	BOOL bAllTimeTask,const QTime& tmBegin,const QTime& tmEnd,__out TodoTaskList &lst)
// {
// 	QString sQ = L"SELECT * FROM tbl_todo WHERE ",sX;
// 	if (bActiveAndDone)
// 	{
// 		if (bAllTimeTask)
// 		{
// 			sQ += L" (1=1) ";
// 		}
// 		else
// 		{
// 			sX.Format(L" (crtime>=%lf) AND (crtime<=%lf)",
// 				tmBegin.SQLDateTime(),tmEnd.SQLDateTime());
// 			sQ += sX;
// 		}
// 	}	
// 	else
// 	{
// 		if (bAllTimeTask)
// 		{
// 			sX.Format(L"(Status = %d)",eStatus);
// 			sQ += sX;
// 		}
// 		else
// 		{
// 			sX.Format(L" (Status = %d) AND (crtime>=%lf) AND (crtime<=%lf)",
// 				eStatus,tmBegin.SQLDateTime(),tmEnd.SQLDateTime());
// 			sQ += sX;
// 		}
// 	}
// 	return ReadTasks(sQ,lst);
// }

BOOL QDBEvents::TodoTask_Get(int nID, ENUM_TODOTASK_TYPE t_type, __out TTodoTask &t)
{
    CStdString sQ;
    sQ.Format(L"SELECT * FROM tbl_todo WHERE (ID=%d)", nID);
    return _TodoTask(ExecQuery(sQ), t_type, t);
}

BOOL QDBEvents::_TodoTask(SqlQuery &q, ENUM_TODOTASK_TYPE t_type, __out TTodoTask&t)
{
    QDB_BEGIN_TRY
        if (!q.eof())
        {
            t.nFlag = q.IntValue(L"Flag");
            if (TODOTASK_TYPE_ALL != t_type)
            {
                switch (t_type)
                {
                case TODOTASK_TYPE_STICKNOTE:
                    if (!_HasFlag(t.nFlag, TODO_FLAG_STICKYNOTE))
                        return FALSE;
                    break;

                case TODOTASK_TYPE_TODO:
                    if (_HasFlag(t.nFlag, TODO_FLAG_STICKYNOTE ))  // 非便签
                        return FALSE;
                    break;
                }
            }

            t.nID = q.IntValue(L"ID");
            t.sTask = q.StrValue(L"Task");
            t.eStatus = (ENUM_TODO_STATUS)q.IntValue(L"Status");
            t.nPriority = q.IntValue(L"Priority");
            t.nCateID = q.IntValue(L"CateID");
            t.tmExec = q.DateTimeValue(L"ETime");
            t.tmCreate = q.DateTimeValue(L"crtime");

            return TRUE;
        }
    QDB_END_TRY
        return FALSE;
}

BOOL QDBEvents::TodoTask_Read(LPCWSTR szSQL, ENUM_TODOTASK_TYPE t_type, __out TodoTaskList & lst)
{
    QDB_BEGIN_TRY
    {
        SqlQuery q = ExecQuery(szSQL);
        for (q.nextRow(); !q.eof(); q.nextRow())
        {
            TTodoTask t;
            if (_TodoTask(q, t_type, t))
            {
                lst.push_back(t);
            }
        }
        return TRUE;
    }
    QDB_END_TRY;
    return FALSE;
}

int QDBEvents::TodoTask_Add(TTodoTask *pItem)
{
    CStdString sQ;
    sQ.Format(L"INSERT INTO tbl_todo(Task,Status,Priority,CateID,Flag,ETime,crtime,udtime) "
        L"VALUES('%s',%d,%d,%d,%d,%lf,%lf,%lf)",
        SFHSQ(pItem->sTask),
        TODO_STATUS_PROCESSING,
        pItem->nPriority,
        pItem->nCateID,
        pItem->nFlag,
        pItem->tmExec.SQLDateTime(),
        QTime::SQLDateTimeCurrent(),
        QTime::SQLDateTimeCurrent());
    return _ExecSQL_RetLastID(sQ);
}

BOOL QDBEvents::TodoTask_SetStatus(int nID, ENUM_TODO_STATUS eStatus)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_todo SET Status=%d,udtime=%lf WHERE (ID=%d)",
        eStatus, QTime::SQLDateTimeCurrent(), nID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::TodoTask_Edit(TTodoTask* pItem)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_todo SET Task='%s',Status=%d,Priority=%d"
        L",CateID=%d,Flag=%d,ETime=%lf,udtime=%lf WHERE (ID=%d)",
        SFHSQ(pItem->sTask), pItem->eStatus, pItem->nPriority,
        pItem->nCateID,
        pItem->nFlag,
        pItem->tmExec.SQLDateTime(),
        QTime::SQLDateTimeCurrent(),
        pItem->nID);
    return _ExecSQL_RetBOOL(sQ);

}

BOOL QDBEvents::TodoTask_SetPriority(int nID, int nPri)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_todo SET Priority=%d,udtime=%lf WHERE (ID=%d)",
        nPri, QTime::SQLDateTimeCurrent(), nID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::TodoTask_Delete(int nID)
{
    CStdString sQ;
    sQ.Format(L"DELETE FROM tbl_todo WHERE (ID=%d)", nID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::TodoTask_SetFlag(int nID, int flag)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_todo SET flag=%d WHERE (ID=%d)", flag, nID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::TodoTask_IsDone(int nID)
{
    TTodoTask t;
    if (TodoTask_Get(nID, TODOTASK_TYPE_ALL, t))
    {
        return t.IsDone();
    }
    return FALSE;
}

int QDBEvents::TodoTask_GetUnfinishNum()
{
    CStdString sql;
    TodoTaskList lst;
    sql.Format(L"SELECT * FROM tbl_todo WHERE Status=%d", TODO_STATUS_PROCESSING);
    if (TodoTask_Read(sql, TODOTASK_TYPE_TODO, lst))
    {
        return lst.size();
    }
    return 0;
}

int QDBEvents::TodoTask_GetFinishedNum()
{
    CStdString sQ;
    sQ.Format(L"SELECT COUNT(ID) FROM tbl_todo WHERE (Status=%d)", TODO_STATUS_FINISH);
    return _ExecSQL_RetInt(sQ);
}

//////////////////////////////////////////////////////////////////////////
// Category
int QDBEvents::Cate_CountTaskInIt(int nCateID)
{
    CStdString sQ;
    sQ.Format(L"SELECT COUNT(ID) FROM tbl_todo WHERE (CateID=%d)", nCateID);
    return _ExecSQL_RetInt(sQ);
}

int QDBEvents::Cate_Add(const CStdStringW &sCate,
    UINT nFlags, int nIconID, const CStdString& sDescription)
{
    CStdString sQ;
    sQ.Format(L"INSERT INTO tbl_catelog(Cate,IconID,Des,crtime,udtime)"
        L" VALUES('%s',%d,'%s',%lf,%lf)",
        SFHSQ(sCate), nIconID, SFHSQ(sDescription),
        QTime::SQLDateTimeCurrent(),
        QTime::SQLDateTimeCurrent());
    return _ExecSQL_RetLastID(sQ);
}

BOOL QDBEvents::Cate_Edit(int nCateID, const CStdStringW &sCate,
    int nIconID, const CStdString& sDescription)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_catelog SET Cate='%s',IconID=%d,Des='%s'"
        L" udtime=%lf WHERE (ID=%d)",
        SFHSQ(sCate), nIconID, SFHSQ(sDescription),
        QTime::SQLDateTimeCurrent(), nCateID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::Cate_Delete(int nCateID)
{
    CStdString sQ;
    sQ.Format(L"DELETE FROM tbl_catelog WHERE (ID=%d)", nCateID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::TodoTask_SetCate(int nTaskID, int nCateID)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_catelog SET CateID=%d WHERE (ID=%d)", nCateID, nTaskID);
    return _ExecSQL_RetBOOL(sQ);
}

int QDBEvents::Cate_CountNum()
{
    return _ExecSQL_RetInt(L"SELECT COUNT(ID) FROM tbl_catelog");
}

BOOL QDBEvents::Cate_IsHasTask(int nCateID)
{
    return Cate_CountTaskInIt(nCateID) > 0;
}

BOOL QDBEvents::Cate_IsExist(const CStdString& sCate)
{
    CStdString sQ;
    sQ.Format(L"SELECT COUNT(ID) FROM tbl_catelog WHERE (Cate='%s')", sCate);
    return _ExecSQL_RetInt(sQ);
}

int QDBEvents::Cate_GetAll(VecCate &vc)
{
    QDB_BEGIN_TRY
        SqlQuery q = ExecQuery(L"SELECT * FROM tbl_catelog");
    for (q.nextRow(); !q.eof(); q.nextRow())
    {
        TCate c;
        if (_Cate(q, c))
        {
            vc.push_back(c);
        }
    }
    QDB_END_TRY
        return vc.size();
}

BOOL QDBEvents::_Cate(SqlQuery &q, __out TCate &c)
{
    QDB_BEGIN_TRY
        if (!q.eof())
        {
            c.nID = q.IntValue(L"ID");
            c.nIconID = q.IntValue(L"IconID");
            c.sCate = q.StrValue(L"Cate");
            c.sDes = q.StrValue(L"Des");
            c.tmCreate = q.DateTimeValue(L"crtime");
            c.tmUpdate = q.DateTimeValue(L"udtime");
            return TRUE;
        }
    QDB_END_TRY
        return FALSE;
}

//////////////////////////////////////////////////////////////////////////
BOOL QDBEvents::AutoTask_GetAll(AutoTaskList & lst)
{
    QDB_BEGIN_TRY
        // 所有的自动计划
        QAutoTask *pTask;
    SqlQuery q = ExecQuery(L"SELECT * FROM tbl_autotask");
    for (q.nextRow(); !q.eof(); q.nextRow())
    {
        pTask = new QAutoTask(
            q.StrValue(L"Task"),
            q.IntValue(L"ID"),
            q.IntValue(L"TimerID"),
            (ENUM_AUTOTASK_DOWHAT)q.IntValue(L"What"),
            q.IntValue(L"Flag"),
            q.DateTimeValue(L"crtime"));
        lst.push_back(pTask);
    }
    return TRUE;
    QDB_END_TRY
        return FALSE;
}

int QDBEvents::AutoTask_Add(LPCWSTR pszTask, int nTimerID, ENUM_AUTOTASK_DOWHAT eDo, int nFlag)
{
    CStdString sQ;
    sQ.Format(L"INSERT INTO "
        L" tbl_autotask(Task,TimerID,What,Flag,crtime,udtime) "
        L" VALUES('%s',%d,%d,%d,%lf,%lf)",
        SFHSQ(pszTask), nTimerID, eDo, nFlag,
        QTime::SQLDateTimeCurrent(), QTime::SQLDateTimeCurrent());
    return _ExecSQL_RetLastID(sQ);
}

BOOL QDBEvents::AutoTask_SetTimer(int nTaskID, int nTimerID)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_autotask SET TimerID=%d,udtime=%lf WHERE(ID=%d)",
        nTimerID, QTime::SQLDateTimeCurrent(), nTaskID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::AutoTask_Edit(int nTaskID, LPCWSTR pszTask,
    ENUM_AUTOTASK_DOWHAT eDo, int nFlag)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_autotask SET "
        L" Task='%s',What=%d,Flag=%d,udtime=%lf"
        L" WHERE(ID=%d)",
        SFHSQ(pszTask), eDo, nFlag, QTime::SQLDateTimeCurrent(), nTaskID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::AutoTask_SetDo(int nTaskID, LPCWSTR pszTask, ENUM_AUTOTASK_DOWHAT eDo)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_autotask SET Task='%s',What=%d,udtime=%lf WHERE(ID=%d)",
        SFHSQ(pszTask), eDo, QTime::SQLDateTimeCurrent(), nTaskID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::AutoTask_GetInfo(int nTaskID,
    __out CStdString & sTask, __out int &nTimerID,
    __out ENUM_AUTOTASK_DOWHAT &eDo, __out int& nFlag,
    __out QTime &tmCreate)
{
    QDB_BEGIN_TRY
        // 所有的自动计划
        CStdString sQ;
    sQ.Format(L"SELECT * FROM tbl_autotask WHERE (ID=%d)", nTaskID);
    SqlQuery q = ExecQuery(sQ);
    if (!q.eof())
    {
        sTask = q.StrValue(L"Task");
        nTimerID = q.IntValue(L"TimerID");
        eDo = (ENUM_AUTOTASK_DOWHAT)q.IntValue(L"What");
        nFlag = q.IntValue(L"Flag");
        tmCreate = q.DateTimeValue(L"crtime");

        return TRUE;
    }
    QDB_END_TRY
        return FALSE;
}

int QDBEvents::AutoTask_GetTimerID(int nTaskID)
{
    CStdString sQ;
    sQ.Format(L"SELECT TimerID FROM tbl_autotask WHERE (ID=%d)", nTaskID);
    return _ExecSQL_RetInt(sQ);
}

BOOL QDBEvents::AutoTask_Delete(int nTaskID)
{
    int nTimerID = AutoTask_GetTimerID(nTaskID);
    if (INVALID_ID != nTimerID)
    {
        // 删除关联的Timer
        CStdString sQ;
        sQ.Format(L"DELETE FROM tbl_timer WHERE (ID=%d)", nTimerID);
        _ExecSQL_RetBOOL(sQ);
        sQ.Format(L"DELETE FROM tbl_autotask WHERE (ID=%d)", nTaskID);
        return _ExecSQL_RetBOOL(sQ);
    }
    return FALSE;
}

BOOL QDBEvents::AutoTask_SetFlag(int nTaskID, int nFlag)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_autotask SET Flag=%d,udtime=%lf WHERE (ID=%d)",
        nFlag, QTime::SQLDateTimeCurrent(), nTaskID);
    return _ExecSQL_RetBOOL(sQ);
}

//////////////////////////////////////////////////////////////////////////
// [ID] INTEGER PRIMARY KEY AUTOINCREMENT DEFAULT (1), 
// [BTime] DOUBLE NOT NULL,
// [ETime] DOUBLE NOT NULL,
// [WhenExp] TEXT,
// [Reminder] TEXT,
// [XFiled] TEXT);
int QDBEvents::Timer_Add(const QTime &tmBegin, const QTime& tmEnd,
    LPCWSTR szWhen, LPCWSTR szReminder, LPCWSTR szXField)
{
    CStdString sQ;
    sQ.Format(L"INSERT INTO "
        L" tbl_timer(BTime,ETime,WhenExp,Reminder,XFiled) "
        L" VALUES(%lf,%lf,'%s','%s','%s')",
        tmBegin.SQLDateTime(), tmEnd.SQLDateTime(),
        SFHSQ(szWhen), SFHSQ(szReminder), SFHSQ(szXField));
    return _ExecSQL_RetLastID(sQ);
}

BOOL QDBEvents::Timer_Delete(int nTimerID)
{
    CStdString sQ;
    sQ.Format(L"DELETE FROM tbl_timer WHERE (ID=%d)", nTimerID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::EditTimer(int nTimerID, const QTime &tmBegin, const QTime& tmEnd,
    LPCWSTR szWhen, LPCWSTR szReminder, LPCWSTR szXField)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_timer SET "
        L"BTime=%lf,ETime=%lf,"
        L"WhenExp='%s',Reminder='%s',XFiled='%s' "
        L" WHERE (ID=%d)",
        tmBegin.SQLDateTime(), tmEnd.SQLDateTime(),
        SFHSQ(szWhen), SFHSQ(szReminder), SFHSQ(szXField), nTimerID);
    return _ExecSQL_RetBOOL(sQ);
}

BOOL QDBEvents::Timer_SetRemindExp(int nTimerID, LPCWSTR pszRmdExp)
{
    CStdString sQ;
    sQ.Format(L"UPDATE tbl_timer SET Reminder='%s' WHERE (ID=%d)",
        SFHSQ(pszRmdExp), nTimerID);
    return _ExecSQL_RetBOOL(sQ);
}

QTimer* QDBEvents::Timer_Get(int nID)
{
    QDB_BEGIN_TRY
    {
        CStdString sQ;
        sQ.Format(L"SELECT * FROM tbl_timer WHERE (ID=%d)", nID);
        SqlQuery q = ExecQuery(sQ);
        if (!q.eof())
        {
            QTimer *pTimer = new QTimer(
                nID,
                q.DateTimeValue(L"BTime"),
                q.DateTimeValue(L"ETime"),
                q.StrValue(L"WhenExp"),
                q.StrValue(L"Reminder"),
                q.StrValue(L"XFiled"));
            return pTimer;
        }
    }
        QDB_END_TRY
        return NULL;
}

BOOL QDBEvents::Timer_GetInfo(int nID, __out QTime&tmBegin, __out QTime&tmEnd,
    __out CStdString &sWhenExp, __out CStdString &sRemindExp, __out CStdString &sXFiled)
{
    QDB_BEGIN_TRY
    {
        CStdString sQ;
        sQ.Format(L"SELECT * FROM tbl_timer WHERE (ID=%d)", nID);
        SqlQuery q = ExecQuery(sQ);
        if (!q.eof())
        {
            tmBegin = q.DateTimeValue(L"BTime");
            tmEnd = q.DateTimeValue(L"ETime");
            sWhenExp = q.StrValue(L"WhenExp");
            sRemindExp = q.StrValue(L"Reminder");
            sXFiled = q.StrValue(L"XFiled");
            return TRUE;
        }
    }
        QDB_END_TRY
        return FALSE;
}


