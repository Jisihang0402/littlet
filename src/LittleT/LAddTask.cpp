#include "LAddTask.h"

#include "../common/LittleTUIcmn.h"
#include "../common/QDBHelper.h"
#include "../common/QHelper.h"
#include "../common/QAutoTask.h"
#include "../common/QTimerMan.h"

#include "ui/ECtrl.h"
#include "ui/QUIMgr.h"
#include "QBuffer.h"
#include "AppHelper.h"

/*
struct TASK_DO
{
	QString sItem;
	ENUM_AUTOTASK_DOWHAT eDo;
}g_mapTaskDo[]=
{
	{L"do_what_reminder",AUTOTASK_DO_REMIND},
	{L"do_what_execprog",AUTOTASK_DO_EXECPROG},
	{L"do_what_sysreboot",AUTOTASK_DO_SYSREBOOT},
	{L"do_what_sysshutdown",AUTOTASK_DO_SYSSHUTDOWN},
};

QString GetTaskDoString(ENUM_AUTOTASK_DOWHAT eDo)
{
	for (int i = 0; i < ARRAYSIZE(g_mapTaskDo); i++)
	{
		if (eDo == g_mapTaskDo[i].eDo)
		{
			return g_mapTaskDo[i].sItem;
		}
	}
	return L"";
}

ENUM_AUTOTASK_DOWHAT GetTaskDo(LPCWSTR sDoString)
{
	for (int i = 0; i < ARRAYSIZE(g_mapTaskDo); i++)
	{
		if (0 == g_mapTaskDo[i].sItem.CompareNoCase(sDoString))
		{
			return g_mapTaskDo[i].eDo;
		}
	}
	return AUTOTASK_DO_NOTSET;
}
*/

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(QExecTimeDlg,QDialog)
	BN_CLICKED_ID(L"chk_span2", &QExecTimeDlg::OnClkChkSpan)
	BN_CLICKED_ID(L"chk_exec_count", &QExecTimeDlg::OnClkChkExecCount)
	BN_CLICKED_ID(L"btn_SelectAll", &QExecTimeDlg::OnClkMonthDaySelectAll)
    BN_CLICKED_ID(L"btn_SelectNone", &QExecTimeDlg::OnClkMonthDaySelectNone)
    BN_CLICKED_ID(L"btn_addtime", &QExecTimeDlg::OnClkAddTimeSpot)
	BN_CLICKED_NAME(L"td_delrow", &QExecTimeDlg::OnClkDeleteTimeRow)
    BN_STATECHANGED_ID(L"rio_absDay",&QExecTimeDlg::OnAbsDateRioCheckChanged)
    BN_STATECHANGED_ID(L"rio_absDaily",&QExecTimeDlg::OnAbsDateRioCheckChanged)
    BN_STATECHANGED_ID(L"rio_absWeek",&QExecTimeDlg::OnAbsDateRioCheckChanged)
    BN_STATECHANGED_ID(L"rio_absMonthDay",&QExecTimeDlg::OnAbsDateRioCheckChanged)
    BN_STATECHANGED_ID(L"rio_absYearDay",&QExecTimeDlg::OnAbsDateRioCheckChanged)
    CMB_SELECTION_CHANGED_ID(L"cmb_task_when_do", &QExecTimeDlg::OnCmbTaskWhenDoSelectChanged)  
    CMB_SELECTION_CHANGED_ID(L"cmb_date_yearmonth", &QExecTimeDlg::OnCmbAbsDateMonthSelectChanged)
//    CMB_SELECTION_CHANGED_ID(L"time_absTime",&QExecTimeDlg::OnSelectDatetimeChanged)
//     QMSG_ID_SELECTCHANGED(L"date_begin", &QExecTimeDlg::OnSelectDatetimeChanged)
//     QMSG_ID_SELECTCHANGED(L"date_end", &QExecTimeDlg::OnSelectDatetimeChanged)
//     QMSG_ID_SELECTCHANGED(L"time_begin", &QExecTimeDlg::OnSelectDatetimeChanged)
//     QMSG_ID_SELECTCHANGED(L"time_end", &QExecTimeDlg::OnSelectDatetimeChanged)
QUI_END_EVENT_MAP()

QExecTimeDlg::QExecTimeDlg()
	:QDialog(L"qabs:addtask/exectime.htm")
{
	m_bEditMode = FALSE;
	life_begin_ = life_end_ = QTime::GetCurrentTime();
}

QExecTimeDlg::QExecTimeDlg( const QTime& tmBegin,
	const QTime& tmEnd,LPCWSTR sExpWhen )
    :QDialog(L"qabs:addtask/exectime.htm")
{
	life_begin_ = tmBegin;
	life_end_ = tmEnd;
	m_sExp = sExpWhen;
	m_bEditMode = TRUE;
}

QExecTimeDlg::~QExecTimeDlg(void)
{
}

void QExecTimeDlg::OnClkChkSpan( HELEMENT hBtn )
{
	if (ECtrl(hBtn).get_state(STATE_CHECKED))
	{		
		GetCtrl("#edit_span2").ShowCtrl(SHOW_MODE_SHOW);
		GetCtrl("#cmb_span_unit2").ShowCtrl(SHOW_MODE_SHOW);
		GetCtrl("#chk_exec_count").ShowCtrl(SHOW_MODE_SHOW);
		if (ECheck(GetCtrl("#chk_exec_count")).IsChecked())
			GetCtrl("#edit_exec_count").ShowCtrl(SHOW_MODE_SHOW);
		else
			GetCtrl("#edit_exec_count").ShowCtrl(SHOW_MODE_HIDE);
	}
	else
	{
		GetCtrl("#edit_span2").ShowCtrl(SHOW_MODE_COLLAPSE);
		GetCtrl("#cmb_span_unit2").ShowCtrl(SHOW_MODE_COLLAPSE);
		GetCtrl("#chk_exec_count").ShowCtrl(SHOW_MODE_COLLAPSE);
		GetCtrl("#edit_exec_count").ShowCtrl(SHOW_MODE_COLLAPSE);
	}
}

void QExecTimeDlg::OnClkChkExecCount( HELEMENT hBtn )
{
	GetCtrl("#edit_exec_count").ShowCtrl(
        ECheck(hBtn).IsChecked()?SHOW_MODE_SHOW:SHOW_MODE_HIDE);
}

void QExecTimeDlg::OnCmbTaskWhenDoSelectChanged(ECombobox cmb, EOption item)
{    
	ENUM_AUTOTASK_EXECFLAG eWhenDo =  
		(ENUM_AUTOTASK_EXECFLAG)(DWORD)item.GetData(); //(DWORD)cmb.GetItemData(cmb.GetCurSel());
	if ((AUTOTASK_EXEC_AFTERPROGEXIT == eWhenDo)
		|| (AUTOTASK_EXEC_AFTERPROGSTART == eWhenDo))
		GetCtrl("#path_progpath").ShowCtrl(SHOW_MODE_SHOW);
	else
		GetCtrl("#path_progpath").ShowCtrl(SHOW_MODE_COLLAPSE);
}

void MakeRelateExecFlagCMB(ECombobox &eCmb)
{
	eCmb.DeleteAllItem();

	EOption op;
	op = eCmb.InsertItem(L"ϵͳ������");
	op.SetData((LPVOID)AUTOTASK_EXEC_AFTERSYSBOOT);
	op = eCmb.InsertItem(L"����������");
	op.SetData((LPVOID)AUTOTASK_EXEC_AFTERTASKSTART);
	op = eCmb.InsertItem(L"������������");
	op.SetData((LPVOID)AUTOTASK_EXEC_AFTERMINDERSTART);
/*	idx = eCmb.InsertItem(QGetEnumFlagString(L"11"));
	eCmb.SetItemData(idx,(LPVOID)TASK_EXEC_AFTERPROGSTART);
	idx = eCmb.InsertItem(QGetEnumFlagString(L"12"));
	eCmb.SetItemData(idx,(LPVOID)TASK_EXEC_AFTERPROGEXIT);
*/
	eCmb.SetCurSel(0);
}

LRESULT QExecTimeDlg::OnDocumentComplete()
{
	QDialog::on_document_complete();
	
	ECombobox eCmbRelateExec = GetCtrl("#cmb_task_when_do");
	MakeRelateExecFlagCMB(eCmbRelateExec);

	// ������
	EDate(GetCtrl("#date_begin")).SetDate(life_begin_);
	EDate(GetCtrl("#date_end")).SetDate(life_end_);
	ETime(GetCtrl("#time_begin")).SetTime(life_begin_);
	ETime(GetCtrl("#time_end")).SetTime(life_end_);

	if (!m_bEditMode)
		return 0;
	// �༭ģʽ���趨�����ؼ���ֵ
	QTimer testTimer;
	if (!testTimer.SetExp(m_sExp))
	{
		ASSERT(FALSE);
		return 0;
	}
	if (testTimer.IsRelateTimer())
	{// timer ģʽ�����ʱ��
        _TabTime().SelectTab(_TabItemRelate());

		// when do
		eCmbRelateExec.SelectItem_ItemData((LPVOID)testTimer.GetExecFlag());
		// program path
		EFilePath(GetCtrl("#path_progpath")).SetFilePath(testTimer.GetXFiled());
		// span
		//-----------------------------------------------------------
		DWORD dwNum = testTimer.GetExecSpan();
		if (dwNum <= 0)
			return 0;
		EEdit(GetCtrl("#edit_span")).SetInt(dwNum);
		// span unit
		WCHAR cUnit = testTimer.GetExecSpanUnit();
		ECombobox(GetCtrl("#cmb_span_unit")).SelectItem_IDorName(&cUnit);
		// span2
		//-----------------------------------------------------------
		dwNum = testTimer.GetExecSpan2();
		if (dwNum <= 0)
			return 0;
		ECheck eChk1 = GetCtrl("#chk_span2");
		eChk1.SetCheck(TRUE);
		OnClkChkSpan(eChk1);
		EEdit(GetCtrl("#edit_span2")).SetInt(testTimer.GetExecSpan2());
		// span2 unit
		cUnit = testTimer.GetExecSpanUnit2();
		ECombobox(GetCtrl("#cmb_span_unit2")).SelectItem_IDorName(&cUnit);
		//-----------------------------------------------------------
		// execute count
		dwNum = testTimer.GetExecCount();
		if (dwNum <= 0)
			return 0;
		ECheck eChk2 = GetCtrl("#chk_exec_count");
		eChk2.SetCheck(TRUE);
		OnClkChkExecCount(eChk2);
		EEdit(GetCtrl("#edit_exec_count")).SetInt(dwNum);
	}
	else	// ABS
	{
		DWORD dw = testTimer.GetExecDate();
		if (dw == 0)
        {
            ASSERT(FALSE);
            return 0;
        }

        // ���� week��monthday
        class chk_cb : public htmlayout::dom::callback
        {
        public:
            DWORD m_v;
            chk_cb(DWORD dwV):m_v(dwV){}
            virtual bool on_element(HELEMENT he)  /* return false to continue enumeration*/
            {
                ECheck chk(he);
                chk.SetCheck( m_v & (0x1 << chk.get_attribute_int("v")));
                return false;
            }
        };

        _TabTime().SelectTab(_TabItemAbs());

		GetCtrl("#rio_absDay").SetCheck(FALSE);
        ECtrl ctl;
		switch (testTimer.GetExecFlag())
		{
		case AUTOTASK_EXEC_ATDAILY:
			{
                ctl = GetCtrl("#rio_absDaily");
                ctl.SetCheck(TRUE);
				EEdit(GetCtrl("#edit_XDay")).SetInt(dw);
				break;
			}
		case AUTOTASK_EXEC_ATDATE:
			{
				ctl = GetCtrl("#rio_absDay");
                ctl.SetCheck(TRUE);
                EDate(GetCtrl("#date_Day")).SetDate(QTime::ParseDate(dw));
				break;
			}
        case AUTOTASK_EXEC_ATYEARDAY:
            {
                ctl = GetCtrl("#rio_absYearDay");
                ctl.SetCheck(TRUE);

                QTime t = QTime::ParseDate(dw);
                ECombobox cmb_month = GetCtrl("#cmb_date_yearmonth");
                cmb_month.SetCurSel(t.GetMonth() - 1);
                ECombobox(GetCtrl("#cmb_date_yearday")).SetCurSel(t.GetDay() - 1);
                OnCmbAbsDateMonthSelectChanged(cmb_month, cmb_month.GetCurSelItem());

                DWORD adv_day = testTimer.GetExecSpan();
                ECheck(GetCtrl("#chk_advance")).SetCheck(0 != adv_day);
                if (0 != adv_day)
                {
                    ENumber(GetCtrl("#edit_daysadv")).SetInt(adv_day);
                }
                break;
            }
		case AUTOTASK_EXEC_ATMONTHDAY:
			{
				ctl = GetCtrl("#rio_absMonthDay");
                ctl.SetCheck(TRUE);
                GetCtrl("#menu_absMonthDay").select_elements(&chk_cb(dw),L".chk_table td");
				break;
			}
		case AUTOTASK_EXEC_ATWEEKDAY:
			{
				ctl = GetCtrl("#rio_absWeek");
                ctl.SetCheck(TRUE);
                GetCtrl("#menu_absWeekday").select_elements(&chk_cb(dw),L".chk_table td");
				break;
			}
		}
        if (ctl.is_valid())
        {
            ctl.post_event(BUTTON_STATE_CHANGED, 0, ctl);
        }


        // ʱ���
        std::vector<QTime> vTimes;
        int nTimeSpots = testTimer.GetExecTimeSpot(vTimes);
        ETable lstTimes = _ListTimes();
        CStdString str;
        for (int i = 0; i < nTimeSpots; ++i)
        {
            str.Format(L"<td>%s</td><td .td_close name=\"td_delrow\">r</td>",
                vTimes[i].Format(L"%H:%M:%S"));
            lstTimes.InsertRow(str);
        }
	}
	return 0;
}

void QExecTimeDlg::GetLifeTime(QTime& tmBegin,QTime& tmEnd)
{
	QTime d1 = EDate(GetCtrl("#date_begin")).GetDate();
	QTime d2 = EDate(GetCtrl("#date_end")).GetDate();
	QTime t1 = ETime(GetCtrl("#time_begin")).GetTime();
	QTime t2 = ETime(GetCtrl("#time_end")).GetTime();

	tmBegin.SetDateTime(d1.GetYear(),d1.GetMonth(),d1.GetDay(),
				t1.GetHour(),t1.GetMinute(),t1.GetSecond());
	tmEnd.SetDateTime(d2.GetYear(),d2.GetMonth(),d2.GetDay(),
				t2.GetHour(),t2.GetMinute(),t2.GetSecond());
}

BOOL QExecTimeDlg::GetRelateExp(__out CStdString & sExp)
{
	// R=1;P=20m;Q=22m;C=100;
	// R
	ECombobox eCmbWhenDo = GetCtrl("#cmb_task_when_do");
    ENUM_AUTOTASK_EXECFLAG eFlag = 
        (ENUM_AUTOTASK_EXECFLAG) (int)(eCmbWhenDo.GetCurSelItem().GetData());
    
	//P - span1
    int nSpan = ENumber(GetCtrl("#edit_span")).GetInt();
	wchar_t cUnit = ECombobox(GetCtrl("#cmb_span_unit")).GetCurSelItemAttribute("name").front();
	DWORD dwSecs = QHelper::HowManySeconds(nSpan,cUnit);
	if ((dwSecs < 30) || (dwSecs > 86400))
	{
		GetCtrl("#edit_span").ShowTooltip(L"���ʱ��������30�벢С��24Сʱ");
		return FALSE;
	}

	// Q - span2
    int nSpan1 = 0;
    wchar_t cUnit1 = L's';
    int nExecCount = 0;
	if (ECheck(GetCtrl("#chk_span2")).IsChecked())
	{ // span2
		nSpan1 = ENumber(GetCtrl("#edit_span2")).GetInt();
		cUnit1 = ECombobox(GetCtrl("#cmb_span_unit2")).GetCurSelItemAttribute("name").front();
		dwSecs = QHelper::HowManySeconds(nSpan1 ,cUnit1);
		if ((dwSecs < 30) || (dwSecs > 86400))
		{
			GetCtrl("#edit_span2").ShowTooltip(L"���ʱ��������30�벢С��24Сʱ");
			return FALSE;
		}

		// C - exec count
		if (ECheck(GetCtrl("#chk_exec_count")).IsChecked())
		{
			nExecCount = ENumber(GetCtrl("#edit_exec_count")).GetInt();
			if (nExecCount <= 0)
			{
				GetCtrl("#edit_exec_count").ShowTooltip(L"���趨������ظ���������Χ[1,~]");
				return FALSE;
			}
		}
	}

    sExp = QHelper::MakeReleateExp(eFlag, nSpan, cUnit, nSpan1, cUnit1, nExecCount);

	return TRUE;
}

BOOL QExecTimeDlg::GetAbsoluteExp( __out CStdString &sExp ,QTime&tmB,QTime &tmE)
{
	GetLifeTime(tmB,tmE);

    // �Ȼ�ȡʱ���
    ENUM_AUTOTASK_EXECFLAG eFlag;
    ETable tblTimes = _ListTimes();
    StringArray arTimePots;
    int nTimesPot = tblTimes.GetRowCount();
    if (nTimesPot > 0)
    {
        CStdString sTimePots;
        for (int i = 0; i < nTimesPot; ++i)
        {
            CStdString s = tblTimes.GetCell(i, 0).GetText();
            if (std::find(arTimePots.begin(), arTimePots.end(), s) == arTimePots.end())
            {
                arTimePots.push_back(s);
            }
            // arTimePots.push_back(tblTimes.GetCellText(i,0));
        }
    }
    else
    {
        tblTimes.ShowTooltip(L"����������һ��ִ��ʱ��");
        return FALSE;
    }

    StringArray arDatePots;
    // ���ڲ���
	if (ERadio(GetCtrl("#rio_absDay")).IsChecked())
	{ // �������ں�ʱ���ִ��
		EDate eDay = GetCtrl("#date_Day");
		QTime d = eDay.GetDate();
        if (d.CompareDate(QTime::GetCurrentTime()) < 0)
		{
            eDay.ShowTooltip(L"Ӧ���趨һ��δ����ʱ��");
			return FALSE;
		}
        eFlag = AUTOTASK_EXEC_ATDATE;
        arDatePots.push_back(d.Format(L"%Y/%m/%d"));
	}
	else if (ERadio(GetCtrl("#rio_absDaily")).IsChecked())
	{ // ÿ��x��ִ��һ��
		ENumber eDay = GetCtrl("#edit_XDay");
		int nXDay = eDay.GetInt();
		if ((nXDay < 1) || (nXDay > 30))
		{
			eDay.ShowTooltip(L"����дһ��[1,30]֮�����");
			return FALSE;
		}
        CStdString sDay = aux::itow(nXDay);
        arDatePots.push_back(sDay);
        eFlag = AUTOTASK_EXEC_ATDAILY;
	}
	else if (ERadio(GetCtrl("#rio_absWeek")).IsChecked())
	{ // ÿ���µĵ�x�ܣ�x-[1,5]�����͵�x�ܵ���y��y-[1,7]��
		// �ܼ�
        class weeks_cb : public htmlayout::dom::callback
        {
        public:
            weeks_cb(StringArray* pSA):m_pSA(pSA){ }
            StringArray *m_pSA;

            virtual bool on_element(HELEMENT he)  /* return false to continue enumeration*/
            {
                m_pSA->push_back(ECtrl(he).get_attribute("v"));
                return false;
            }
        }wcb(&arDatePots);
        GetCtrl("#div_dates>#menu_absWeekday").select_elements(&wcb, L"td:checked");
		if (arDatePots.empty())
		{
			GetCtrl("#td_absWeekday").ShowTooltip(L"����Ҫѡ��һ��");
			return FALSE;
		}
        eFlag = AUTOTASK_EXEC_ATWEEKDAY;
	}
	else if (ERadio(GetCtrl("#rio_absMonthDay")).IsChecked())
	{
		class days_cb : public htmlayout::dom::callback
        {
        public:
            days_cb(StringArray *pSA):m_pSA(pSA){}
            StringArray *m_pSA;
            virtual bool on_element(HELEMENT he)  /* return false to continue enumeration*/
            {
                ECheck chk(he);
                if (chk.IsChecked())
                {
                    m_pSA->push_back(chk.GetText());
                }
                return false;
            }
        }dcb(&arDatePots);
        GetCtrl("#div_dates #menu_absMonthDay").select_elements(&dcb, L"td");
        if (arDatePots.empty())
		{
            GetCtrl("#td_absMonthDay").ShowTooltip(L"����Ҫѡ��һ��");
			return FALSE;
		}
        eFlag = AUTOTASK_EXEC_ATMONTHDAY;
	}
    // ÿ���ĳ�졢����������
    else if (ERadio(GetCtrl("#rio_absYearDay")).IsChecked())
    {
        // ��ǰ����
        eFlag = AUTOTASK_EXEC_ATYEARDAY;

        int adv_day = 0;
        if (ECheck(GetCtrl("#chk_advance")).IsChecked())
        {
            adv_day = ENumber(GetCtrl("#edit_daysadv")).GetNum();
        }

        int month = ECombobox(GetCtrl("#cmb_date_yearmonth")).GetCurSelIndex() + 1;
        int day = ECombobox(GetCtrl("#cmb_date_yearday")).GetCurSelIndex() + 1;
        QTime tx;
        tx.SetDateTime(2013, month, day, 0, 0, 0);
        CStdString s;
        s.Format(L"%d", tx.MakeDate());
        arDatePots.push_back(s);
        // �ڶ���ֵ����ǰ����Ϊ0��û����ǰ��
        s.Format(L"%d", adv_day);
        arDatePots.push_back(s);
    }

    CStdString sError;
    if (!QHelper::MakeAbsExp(eFlag, tmB, tmE, arDatePots, arTimePots, sExp, sError))
    {
        XMsgBox::ErrorMsgBox(sError);
        return FALSE;
    }
    return TRUE;
}

BOOL QExecTimeDlg::OnDefaultButton( INT_PTR nID )
{
	if (IDOK != nID)
		return TRUE;

    if (_TabTime().GetCurrentTab() == _TabItemRelate()) 
//	if (ERadioBox(GetCtrl("#rio_exectype_relate")).IsChecked())
	{// relate
		GetLifeTime(life_begin_,life_end_);
		if (!GetRelateExp(m_sExp))
			return FALSE;
	}
	else	// ABS
	{
		if (!GetAbsoluteExp(m_sExp,life_begin_,life_end_))
			return FALSE;
	}
	QTimer testTimer;
	if (!testTimer.SetLifeTime(life_begin_,life_end_))
	{
		GetCtrl("#date_end").ShowTooltip(L"Ҫ�趨һ����Ч�Ŀ�ʼ�ͽ���ʱ��");
		return FALSE;
	}
	if (!testTimer.SetExp(m_sExp))
		return FALSE;

    switch (testTimer.TestStart())
    {
    case AUTOTASK_RUNNING_STATUS_OVERDUE:
    case AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC:
        {
            XMsgBox::ErrorMsgBox(L"�趨�Ķ�ʱʱ������޷�ִ�е������顣");
            return FALSE;
        }
    }
	//////////////////////////////////////////////////////////////////////////
	// for test
	// 		QTime tmNext = QTime::GetCurrentTime();
	// 		while (TASK_RUNNING_STATUS_OK == 
	// 			testTimer.GetNextExecTimeFrom(tmNext,tmNext))
	// 		{
	// 			TRACE(tmNext.Format(L"%c %w\n"));
	// 			tmNext += QTimeSpan(0,0,0,1);
	// 		}
	// 		return FALSE;
	// 		//////////////////////////////////////////////////////////////////////////
	return TRUE;
}

void QExecTimeDlg::OnSelectDatetimeChanged( HELEMENT)
{
// 	QTime tb,te; GetLifeTime(tb,te);
// 	ECtrl eRioAbs = GetCtrl("#rio_exectype_abs");
// 	QString sText;
// 	sText.Format(L"��[<b .yellow>%s</b>,<b .green>%s</b>]ʱ�����",
// 			tb.Format(L"%Y/%m/%d %X"),
// 			te.Format(L"%Y/%m/%d %X"));
// 	utf8::ostream o;
// 	o<<sText;
// 	eRioAbs.set_html(o.data(),o.length());
}

void QExecTimeDlg::OnClkMonthDaySelectAll( HELEMENT hBtn )
{
    class select_all_cb : public htmlayout::dom::callback
    {
    public:
        virtual bool on_element(HELEMENT he)  /* return false to continue enumeration*/
        {
            ECheck(he).SetCheck(TRUE);
            return false;
        }
    };

    GetCtrl("#menu_absMonthDay").select_elements(&select_all_cb(),L"td");
}

void QExecTimeDlg::OnClkMonthDaySelectNone( HELEMENT hBtn )
{
    class select_none_cb : public htmlayout::dom::callback
    {
    public:
        virtual bool on_element(HELEMENT he)  /* return false to continue enumeration*/
        {
            ECheck(he).SetCheck(FALSE);
            return false;
        }
    };

    GetCtrl("#menu_absMonthDay").select_elements(&select_none_cb(),L"td");
}

void QExecTimeDlg::OnClkAddTimeSpot( HELEMENT hBtn )
{
//    ETable ctlList = _ListTimes();
//     QString sTime;
//     ETime ctlTime = GetCtrl("#time_absTime");
//     QTime tTime = ctlTime.GetTime();
//     sTime = tTime.Format(L"%H:%M:%S");
//     for (int i = 0; i < ctlList.GetRowCount(); ++i)
//     {
//         if (0 == ctlList.GetCellText(i,0).CompareNoCase(sTime))
//         {
//             // �Ѵ��ڴ�ʱ���
//             ctlTime.ShowTooltip(L"ʱ����Ѵ���");
//             return;
//         }
//     }
    ESheet cSheet = _ListTimes();
    ETableRow cItem = cSheet.InsertRow(
        L"<td>0:0:0</td><td .td_close name=\"td_delrow\">r</td>");
    cItem.ShowTooltip(L"˫���༭ִ��ʱ��");
//    cSheet.ShowCellWidget(cSheet.GetRowIndex(cItem),0);

// 
//     QString str;
//     str.Format(L"<td>%s</td><td .td_close name=\"td_delrow\">r</td>",sTime);
//     ctlList.InsertRow(str);
}

void QExecTimeDlg::OnClkDeleteTimeRow( HELEMENT hBtn )
{
    ETableRow tRow = ECtrl(hBtn).parent();
    ETable ctlList = _ListTimes();
    ctlList.DeleteRow(ctlList.GetRowIndex(tRow));
}

void QExecTimeDlg::OnAbsDateRioCheckChanged( HELEMENT he)
{
    class hide_all : public htmlayout::dom::callback
    {
    public:
        virtual bool on_element(HELEMENT he)  /* return false to continue enumeration*/
        {
            ECtrl(he).ShowCtrl(SHOW_MODE_COLLAPSE);
            return false;
        }
    };
    GetCtrl("#div_dates").select_elements(&hide_all(), ":root>*");

    CStdString sID = ECtrl(he).ID();
    if (sID.CompareNoCase(L"rio_absDay") == 0)
    {
        GetCtrl("#div_dates>#div_day").ShowCtrl(SHOW_MODE_SHOW);
    }
    else if (sID.CompareNoCase(L"rio_absDaily") == 0)
    {
        GetCtrl("#div_dates>#div_xday").ShowCtrl(SHOW_MODE_SHOW);
    }
    else if (sID.CompareNoCase(L"rio_absWeek") == 0)
    {
        GetCtrl("#div_dates>#menu_absWeekday").ShowCtrl(SHOW_MODE_SHOW);
    }
    else if (sID.CompareNoCase(L"rio_absMonthDay") == 0)
    {
        GetCtrl("#div_dates>#div_monthday").ShowCtrl(SHOW_MODE_SHOW);
    }
    else if (sID.CompareNoCase(L"rio_absYearDay") == 0)
    {
        GetCtrl("#div_dates>#div_yearday").ShowCtrl(SHOW_MODE_SHOW);
    }
}

void QExecTimeDlg::OnCmbAbsDateMonthSelectChanged( ECombobox cmb, EOption item )
{
    ECombobox cmb_day = GetCtrl("#cmb_date_yearday");
    int idx = cmb_day.GetCurSelIndex();
    switch (cmb.GetCurSelIndex() + 1)
    {
    case 1: case 3:case 5: case 7:case 8: case 10:case 12:
        cmb_day.GetItem(30).ShowCtrl(SHOW_MODE_SHOW);
        cmb_day.GetItem(29).ShowCtrl(SHOW_MODE_SHOW);
        break;
    case 4:case 6: case 9:case 11:
        cmb_day.GetItem(29).ShowCtrl(SHOW_MODE_SHOW);
        cmb_day.GetItem(30).ShowCtrl(SHOW_MODE_COLLAPSE);
        if (idx == 30)
        {
            cmb_day.SetCurSel(0);
        }
        break;
    case 2:
        cmb_day.GetItem(29).ShowCtrl(SHOW_MODE_COLLAPSE);
        cmb_day.GetItem(30).ShowCtrl(SHOW_MODE_COLLAPSE);
        if (idx >= 29)
        {
            cmb_day.SetCurSel(0);
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
QUI_BEGIN_EVENT_MAP(LAddEventDlg,QDialog)
	BN_CLICKED_ID(L"chk_TaskRemind", &LAddEventDlg::OnClkChkTaskRemind)
	BN_CLICKED_ID(L"btn_ExecTime", &LAddEventDlg::OnClkExecTime)
	BN_CLICKED_ID(L"chk_remind_playsound", &LAddEventDlg::OnClkChkRemindPlaySound)
	BN_CLICKED_ID(L"chk_remind_customsg", &LAddEventDlg::OnClkChkRemindCustomMsg)
    CMB_SELECTION_CHANGED_ID(L"cmb_TaskDoWhat", &LAddEventDlg::OnSelectChangedDoWhat)
QUI_END_EVENT_MAP()

BOOL LAddEventDlg::EditEvent( QAutoTask *pTask )
{
    LAddEventDlg *p = GetInstance();
    if (p->IsWindow())
    {
        return FALSE;
    }

    p->m_pTask = pTask;
    p->m_bEditMode = TRUE;
    ASSERT(pTask != NULL);
    ASSERT(pTask->GetTimer()); 
    // �ڱ༭�����ڼ䣬��������ͣ�䶨ʱ��
    ASSERT(!pTask->GetTimer()->IsStarted());

    return p->DoModal() == IDOK;
}

QAutoTask* LAddEventDlg::NewEvent()
{
    LAddEventDlg *p = GetInstance();
    if (p->IsWindow())
    {
        return NULL;
    }

    p->m_bEditMode = FALSE;

    return (p->DoModal() == IDOK) ? p->m_pTask : NULL;
}

void LAddEventDlg::OnSelectChangedDoWhat( ECombobox cmb, EOption item )
{
	ENUM_AUTOTASK_DOWHAT eDo = (ENUM_AUTOTASK_DOWHAT)(DWORD)item.GetData(); 
   
    _InputRemindText().ShowCtrl(SHOW_MODE_COLLAPSE);
    GetCtrl("#path_DoWhatExecprog").ShowCtrl(SHOW_MODE_COLLAPSE);

    if (AUTOTASK_DO_EXECPROG == eDo)
    {
        GetCtrl("#path_DoWhatExecprog").ShowCtrl(SHOW_MODE_SHOW);
        GetCtrl("#path_DoWhatExecprog").SetFocus();
    }
	else if (AUTOTASK_DO_REMIND == eDo)
    {
        _InputRemindText().ShowCtrl(SHOW_MODE_SHOW);
        _InputRemindText().SetFocus();
    }
}

void MakeTaskDoWhatCMB(ECombobox &eCmb)
{
    EOption op;

    op = eCmb.InsertItem(L"��ʾ��Ϣ");
    op.SetData((LPVOID)AUTOTASK_DO_REMIND);
    op = eCmb.InsertItem(L"ִ�г���");
    op.SetData((LPVOID)AUTOTASK_DO_EXECPROG);
    // 	idx = eCmb.InsertItem(L"����ϵͳ");
    // 	eCmb.SetItemData(idx,(LPVOID)AUTOTASK_DO_SYSREBOOT);
    op = eCmb.InsertItem(L"�ر�ϵͳ");
    op.SetData((LPVOID)AUTOTASK_DO_SYSSHUTDOWN);
    op = eCmb.InsertItem(L"��Ϣһ���");
    op.SetData((LPVOID)AUTOTASK_DO_BREAKAMOMENT);

    eCmb.SetCurSel(0);
}

LRESULT LAddEventDlg::OnDocumentComplete()
{
	MakeTaskDoWhatCMB(ECombobox(GetCtrl("#cmb_TaskDoWhat")));
	
	if (m_bEditMode)
	{
		ASSERT(m_pTask != NULL);

		SetDoWhat(m_pTask->GetDoWhat(),m_pTask->Task());
		SetRemindExp(m_pTask->RemindExp());
        RefreshWhenDo(m_pTask->GetTimer());
	}

	return 0;
}

void LAddEventDlg::OnClkChkTaskRemind( HELEMENT hBtn)
{
	root_element r(m_hWnd);
	element eBtn(hBtn);
	
	bool bDisable = !eBtn.get_state(STATE_CHECKED);
	element(r.find_first("[id=\"chk_remind_playsound\"]")).toggle_state(STATE_DISABLED,bDisable);
	element(r.find_first("[id=\"filepath_remind_sound\"]")).toggle_state(STATE_DISABLED,bDisable);
	element(r.find_first("[id=\"chk_remind_customsg\"]")).toggle_state(STATE_DISABLED,bDisable);
	element(r.find_first("[id=\"edit_remind_customsg\"]")).toggle_state(STATE_DISABLED,bDisable);
}

void LAddEventDlg::OnClkExecTime( HELEMENT )
{
	QExecTimeDlg *pDlg = NULL;
	if (m_bEditMode)
	{
		pDlg = new QExecTimeDlg(m_pTask->LifeBegin(),
						m_pTask->LifeEnd(),
						m_pTask->TimerExp());
	}
	else
	{
        if (!m_sTimerExp.IsEmpty())
		    pDlg = new QExecTimeDlg(m_tmLifeBegin, m_tmLifeEnd, m_sTimerExp);
        else
            pDlg = new QExecTimeDlg();
	}
	if (pDlg->DoModal() == IDOK)
	{
		m_tmLifeBegin = pDlg->life_begin_;
		m_tmLifeEnd = pDlg->life_end_;
		m_sTimerExp = pDlg->m_sExp;
		QTimer testTimer;
		testTimer.SetExp(m_sTimerExp);
		testTimer.SetLifeTime(m_tmLifeBegin,m_tmLifeEnd);

        RefreshWhenDo(&testTimer);
	}
	delete pDlg;
}

void LAddEventDlg::RefreshWhenDo(QTimer* pTimer)
{
    ASSERT(NULL != pTimer);
    CStdString str = L"<b .yellow>[" + pTimer->GetLifeBegin().Format(L"%c") 
        + pTimer->GetLifeEnd().Format(L" ~ %c") + L"]</b>";
    CStdString sTmp;
    if (pTimer->GetWhenDoString(sTmp))
    {
        str += sTmp;
    }
    str.Replace(L"<br/>",L"");
    str.Replace(L"<br />",L"");
    GetCtrl("#td_ExecTime").SetHtml(str);
}

BOOL LAddEventDlg::OnDefaultButton(INT_PTR nID)
{
	if (IDOK != nID)
		return TRUE;

	// ��ʲô
	CStdString sTask;
	if (!GetTaskString(sTask))
		return FALSE;

	// ��ʾ��Ϣ
	CStdString sRmdExp;
	if (!GetRemindExp(sRmdExp))
		return FALSE;

	QAutoTaskMan* pTaskMgr = QAutoTaskMan::GetInstance();
	if (m_bEditMode)
	{
        QTimerMan *pTimerMgr = QTimerMan::GetInstance();
		QTimer* pTimer = m_pTask->GetTimer();
		ASSERT(!pTimer->IsStarted());
		// edit task
		BOOL bOK = m_pTask->Edit(sTask,GetDoWhat(),m_pTask->Flag());
		if (bOK)
		{
			if (!m_sTimerExp.IsEmpty())
			{ // edit timer
				pTimerMgr->EditTimer(pTimer,m_tmLifeBegin,
					m_tmLifeEnd,m_sTimerExp,sRmdExp,sTask);
			}
			else if (pTimer->GetRemindExp() != sRmdExp)
			{
				pTimerMgr->SetRemindExp(pTimer,sRmdExp);
			}
		}
		else
		{
			ASSERT(FALSE);
			XMsgBox::ErrorMsgBox(L"�༭����ʧ�ܣ�");
            return FALSE;
		}

        ENUM_AUTOTASK_RUNNING_STATUS eStatus = pTimer->TestStart();
        if ((AUTOTASK_RUNNING_STATUS_OVERDUE == eStatus)
            || (AUTOTASK_RUNNING_STATUS_NOCHANCETOEXEC == eStatus))
        {
            XMsgBox::ErrorMsgBox(L"��ʱʱ���޷�ִ�е��������衣");
            return FALSE;
        }
	}
	else
	{ // Add;
        if (m_sTimerExp.IsEmpty())
        {
            GetCtrl("#btn_ExecTime").ShowTooltip(L"��û�趨ִ��ʱ��");
            return FALSE;
        }
        CStdString sError;
        m_pTask = pTaskMgr->NewAutoTask(GetDoWhat(), sTask, m_sTimerExp, 
                    sRmdExp, m_tmLifeBegin, m_tmLifeEnd, sError);
        if (NULL == m_pTask)
        {
            XMsgBox::ErrorMsgBox(sError);
            return FALSE;
        }
	}
	return TRUE;
}

ENUM_AUTOTASK_DOWHAT LAddEventDlg::GetDoWhat()
{
	ECombobox eDo = GetCtrl("#cmb_TaskDoWhat");
	EOption op = eDo.GetCurSelItem();
	return (ENUM_AUTOTASK_DOWHAT)(DWORD)op.GetData();
}

BOOL LAddEventDlg::GetTaskString(CStdString &sTask)
{
	switch (GetDoWhat())
	{
	case AUTOTASK_DO_REMIND:
		{
			EEdit eText = _InputRemindText(); 
			sTask = eText.GetText();
			sTask = sTask.Trim();
			if (sTask.IsEmpty())
			{
				eText.ShowTooltip(L"����д��ʾ��Ϣ");
				return FALSE;
			}
			return TRUE;
		}
	case AUTOTASK_DO_EXECPROG:
		{
			EFilePath ePath = GetCtrl("#path_DoWhatExecprog"); 
			sTask = ePath.GetFilePath();
			if (sTask.IsEmpty())
			{
				ePath.ShowTooltip(L"��ѡ��һ����ִ�г���");
				return FALSE;
			}
			return TRUE;
		}
	case AUTOTASK_DO_NOTSET:
		{
			ASSERT(FALSE);
			return FALSE;
		}
	}
	return  TRUE;
}

BOOL LAddEventDlg::GetRemindExp(CStdString &sRmdExp)
{
    CStdString sUnit,sSound,sMsg;
	if (!GetCtrl("#chk_TaskRemind").IsChecked())
    {
        if (GetDoWhat() == AUTOTASK_DO_REMIND)
        {   // �����������ĳ�£���ôΪ������һ��Ĭ�ϵ�
            // ������ִ��ǰ30����ʾ
            sSound = quibase::GetModulePath() + L"sound\\msg.wav";
            sSound.Replace(L'/',L'\\');
            sUnit = L"S";
            GetTaskString(sMsg);
            sRmdExp = QHelper::MakeRemindExp(30, L'S', sSound, sMsg);
        }
        return TRUE;
    }

	EEdit eTime = GetCtrl("#edit_RemindTime");
	int nTime = eTime.GetInt();
	if (nTime <= 0)
	{
		eTime.ShowTooltip(L"����дһ��ʱ����ֵ����Χ[1~1500]");
		return FALSE;
	}
	ECombobox eUnit = GetCtrl("#cmb_RemindTimeUnit");
	sUnit = eUnit.GetCurSelItemAttribute("name");
	ASSERT(!sUnit.IsEmpty());
	
	// play sound
	if (GetCtrl("#chk_remind_playsound").IsChecked())
	{
		EFilePath eFile = GetCtrl("#filepath_remind_sound");
		sSound = eFile.GetFilePath();
		if (sSound.IsEmpty())
		{
			eFile.ShowTooltip(L"��ѡ��һ�������ļ�(*.wav)");
			return FALSE;
		}
	}
	// custom msg
	if (GetCtrl("#chk_remind_customsg").IsChecked())
	{
		EEdit eInput = GetCtrl("#edit_remind_customsg");
		sMsg = eInput.GetText();
		if (sMsg.IsEmpty())
		{
			eInput.ShowTooltip(L"����д��ʾ��Ϣ");
			return FALSE;
		}
	}
	sRmdExp = QHelper::MakeRemindExp(nTime,sUnit[0],sSound,sMsg);
	return TRUE;
}

void LAddEventDlg::OnClkChkRemindPlaySound( HELEMENT hBtn )
{
	GetCtrl("#filepath_remind_sound").ShowCtrl(
        ECheck(hBtn).IsChecked()? SHOW_MODE_SHOW : SHOW_MODE_HIDE);
}

void LAddEventDlg::OnClkChkRemindCustomMsg( HELEMENT hBtn )
{
	GetCtrl("#edit_remind_customsg").ShowCtrl(
        ECheck(hBtn).IsChecked()? SHOW_MODE_SHOW : SHOW_MODE_HIDE);
}

void LAddEventDlg::SetDoWhat( ENUM_AUTOTASK_DOWHAT eDo ,const CStdString& sTask )
{
	ECombobox eCmbDoWhat = GetCtrl("#cmb_TaskDoWhat");
	eCmbDoWhat.SelectItem_ItemData((LPVOID)eDo);
	switch (eDo)
	{
	case AUTOTASK_DO_REMIND: 
		{ 
			_InputRemindText().SetText(sTask);
			break;
		}
	case AUTOTASK_DO_EXECPROG:
		{
			EFilePath(GetCtrl("#path_DoWhatExecprog")).SetFilePath(sTask);
			break;
		}
	}
 
    EOption opotion = eCmbDoWhat.GetCurSelItem();
	OnSelectChangedDoWhat(eCmbDoWhat, opotion);
}

void LAddEventDlg::SetRemindExp( const CStdString &sRmdExp )
{
	ECheck eRmd = GetCtrl("#chk_TaskRemind");
	CStdString sSound,sMsg;
	int nTime;
	WCHAR cAUnit;
	if (!QTimer::ParseRemindExp(sRmdExp,nTime,cAUnit,sSound,sMsg))
	{
		eRmd.SetCheck(FALSE);
		OnClkChkTaskRemind(eRmd);

		return;
	}
	// time
	EEdit(GetCtrl("#edit_RemindTime")).SetInt(nTime);
	// time unit;
	ECombobox(GetCtrl("#cmb_RemindTimeUnit")).SelectItem_IDorName(&cAUnit);
	// play sound
	if (!sSound.IsEmpty())
	{
		ECheck eChkSound = GetCtrl("#chk_remind_playsound");
		eChkSound.SetCheck(TRUE);
		// sound path
		EFilePath(GetCtrl("#filepath_remind_sound")).SetFilePath(sSound);

		OnClkChkRemindPlaySound(eChkSound);
	}
	// custom msg
	if (!sMsg.IsEmpty())
	{
		ECheck eChkMsg = GetCtrl("#chk_remind_customsg");
		eChkMsg.SetCheck(TRUE);
		// message 
		GetCtrl("#edit_remind_customsg").SetText(sMsg);

		OnClkChkRemindCustomMsg(eChkMsg);
	}
}

