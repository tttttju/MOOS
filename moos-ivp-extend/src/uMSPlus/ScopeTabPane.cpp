///////////////////////////////////////////////////////////////////////////
//
//   MOOS - Mission Oriented Operating Suite 
//  
//   A suit of Applications and Libraries for Mobile Robotics Research 
//   Copyright (C) 2001-2005 Massachusetts Institute of Technology and 
//   Oxford University. 
//    
//   This software was written by Paul Newman at MIT 2001-2002 and Oxford 
//   University 2003-2005. email: pnewman@robots.ox.ac.uk. 
//      
//   This file is part of a  MOOS Utility Component. 
//        
//   This program is free software; you can redistribute it and/or 
//   modify it under the terms of the GNU General Public License as 
//   published by the Free Software Foundation; either version 2 of the 
//   License, or (at your option) any later version. 
//          
//   This program is distributed in the hope that it will be useful, 
//   but WITHOUT ANY WARRANTY; without even the implied warranty of 
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
//   General Public License for more details. 
//            
//   You should have received a copy of the GNU General Public License 
//   along with this program; if not, write to the Free Software 
//   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 
//   02111-1307, USA. 
//
//////////////////////////    END_GPL    //////////////////////////////////
// ScopeTabPane.cpp: implementation of the CScopeTabPane class.


//


//////////////////////////////////////////////////////////////////////


#ifdef _WIN32
#pragma warning(disable : 4786)
#endif

#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Light_Button.H>
#include <string>
#include <algorithm>
#include <cctype>
#include "ScopeTabPane.h"
#include <FL/Fl_Tabs.H>
#include "MOOS/libMOOS/Utils/MOOSUtilityFunctions.h"

#define FONT_SIZE 11

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace
{
class SearchSuggestionWindow : public Fl_Menu_Window
{
public:
    SearchSuggestionWindow():Fl_Menu_Window(1,1)
    {
        set_override();
        end();
    }

    void value(const std::string& sText)
    {
        m_sText = sText;
        fl_font(labelfont(), labelsize());
        int W = w();
        int H = h();
        fl_measure(m_sText.c_str(), W, H, Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_TOP|FL_ALIGN_WRAP));
        size(W+10, H+10);
        redraw();
    }

    void draw()
    {
        draw_box(FL_BORDER_BOX, 0, 0, w(), h(), Fl_Color(175));
        fl_color(FL_BLACK);
        fl_font(labelfont(), 10);
        fl_draw(m_sText.c_str(), 3, 3, w()-6, h()-6, Fl_Align(FL_ALIGN_LEFT|FL_ALIGN_TOP|FL_ALIGN_WRAP));
    }

private:
    std::string m_sText;
};

class CaseInsensitiveLess
{
public:
    bool operator()(const std::string& a, const std::string& b) const
    {
        size_t nMin = std::min(a.size(), b.size());
        for(size_t idx = 0; idx < nMin; ++idx)
        {
            int ca = std::toupper((unsigned char)a[idx]);
            int cb = std::toupper((unsigned char)b[idx]);
            if(ca < cb)
                return true;
            if(ca > cb)
                return false;
        }
        return a.size() < b.size();
    }
};
}

class CScopeTabPane::SearchAutoCompleteInput : public Fl_Input
{
public:
    SearchAutoCompleteInput(int X, int Y, int W, int H, const char* l, CScopeTabPane* pOwner)
        :Fl_Input(X, Y, W, H, l), m_pOwner(pOwner)
    {
    }

    virtual int handle(int e)
    {
        if(e == FL_KEYDOWN && Fl::event_key() == FL_Tab && !(Fl::event_state() & FL_SHIFT))
        {
            if(m_pOwner)
            {
                m_pOwner->HandleSearchAutocomplete();
            }
            return 1;
        }

        int nResult = Fl_Input::handle(e);

        if(m_pOwner)
        {
            if(e == FL_KEYDOWN)
            {
                int nKey = Fl::event_key();
                if(nKey != FL_Tab)
                {
                    m_pOwner->ResetSearchAutocompleteState();
                }
            }
            else if(e == FL_PASTE || e == FL_CUT)
            {
                m_pOwner->ResetSearchAutocompleteState();
            }

            if(e == FL_UNFOCUS)
            {
                m_pOwner->HideSearchSuggestions();
            }
        }

        return nResult;
    }

private:
    CScopeTabPane* m_pOwner;
};

CScopeTabPane::~CScopeTabPane()
{
    StopTimer();
    if(m_pSearchSuggestionWindow)
    {
        m_pSearchSuggestionWindow->hide();
        delete m_pSearchSuggestionWindow;
        m_pSearchSuggestionWindow = NULL;
    }
}


CScopeTabPane::CScopeTabPane( int X, int Y, int W, int H,  char *l ) :BASE(X,Y,W,H,l)
{
    m_FetchThread.Initialise(FetchWorker,this);
    m_FetchThread.Start();

    label(l);
    m_sHost = "LOCALHOST";
    m_lPort = 9000;



    m_nCounts=0;

    m_pSearchInput = NULL;
    m_pSearchSuggestionWindow = NULL;
    m_LastAutocompletePrefix.clear();
    m_AutocompleteCandidates.clear();


    int LHS = X+10;
    int TOP = Y+10;
    int RHS = X+W-10;
    int SEARCH_H = 25;
    int SEARCH_GAP = 5;
    int GRID_H =  (2*H)/3 - SEARCH_H - SEARCH_GAP;
    if(GRID_H < 0)
        GRID_H = 0;
    int GRID_TOP = TOP + SEARCH_H + SEARCH_GAP;
    int BOTTOM_GRID = GRID_TOP+GRID_H;
    int PROC_W = int(0.22*W);
    int PROC_H = (Y+H)-BOTTOM_GRID-10;


    m_pSearchInput = new SearchAutoCompleteInput(LHS, TOP, W-20, SEARCH_H, "Search", this);
    m_pSearchInput->textsize(FONT_SIZE);
    m_pSearchInput->tooltip("Filter variables by name");
    m_pSearchInput->when(FL_WHEN_CHANGED | FL_WHEN_ENTER_KEY_ALWAYS);
    SetID(m_pSearchInput, ID_SEARCH);

    m_pScopeGrid = new CScopeGrid( LHS, GRID_TOP, W-20,GRID_H, "DB" );
    m_pScopeGrid->SetDBImage(&m_DBImage);
    m_pScopeGrid->SetComms(&m_Comms);


    Fl_Group *pC = new Fl_Group(X,BOTTOM_GRID,W,(Y+H)-BOTTOM_GRID);

    {



        //the process list
        m_pProcessList = new Fl_Check_Browser( LHS,BOTTOM_GRID+10,PROC_W,PROC_H,"Processes");
        SetID(m_pProcessList,ID_PROCESS);
        m_pProcessList->tooltip("Click a process name to examine subscriptions and publications");
        m_pProcessList->textsize(FONT_SIZE);
        m_pProcessList->when(FL_WHEN_RELEASE);

        //the subscriber list
        m_pSubscribeList = new Fl_Browser( m_pProcessList->x()+PROC_W+30,BOTTOM_GRID+10,PROC_W,PROC_H,"Subscribes");
        m_pSubscribeList->textsize(FONT_SIZE);

        //the publish list
        m_pPublishList = new Fl_Browser(  m_pSubscribeList->x()+PROC_W+5,BOTTOM_GRID+10,PROC_W,PROC_H,"Publishes");
        m_pPublishList->textsize(FONT_SIZE);


        Fl_Button* pLB = new Fl_Button(m_pPublishList->x()+PROC_W+5,m_pPublishList->y(),15,20,"?");
        pLB->type(FL_TOGGLE_BUTTON);
        pLB->tooltip("show ? (pending) DB entries");
        SetID(pLB,ID_SHOW_PENDING);


        Fl_Group *pMOOSParams = new Fl_Group(RHS-180,BOTTOM_GRID+10,180,100);
        {
            //MOOS parameters - HOST

            m_pDBHostInput = new Fl_Input(    RHS-180,
                BOTTOM_GRID+10,
                100,25,"HostName");


            m_pDBHostInput->align(FL_ALIGN_RIGHT );
            m_pDBHostInput->textsize(FONT_SIZE);        
            m_pDBHostInput->tooltip("name or IP address of machine hosting DB");


            //MOOS parameters - Port
            m_pDBPortInput = new Fl_Int_Input(    m_pDBHostInput->x(),
                m_pDBHostInput->y()+m_pDBHostInput->h()+5,
                100,25,"Port");


            m_pDBPortInput->align(FL_ALIGN_RIGHT );
            m_pDBPortInput->textsize(FONT_SIZE);
            m_pDBPortInput->tooltip("Port Number MOOSDB is listening on");



            m_pConnectButton= new Fl_Button( m_pDBPortInput->x(),
                m_pDBPortInput->y()+m_pDBPortInput->h()+5,
                160,25,"Connect");


            SetID(m_pConnectButton,ID_CONNECT);
            m_pConnectButton->tooltip("Connect to a MOOSDB");

            //set up MOOS values
            m_pDBHostInput->value(m_sHost.c_str());
            m_pDBPortInput->value(MOOSFormat("%ld",m_lPort).c_str());

        }

        pMOOSParams->resizable(0);
    }


    pC->resizable(this);
    end();

    //make things initially grey...
    m_pScopeGrid->deactivate();
    m_pSubscribeList->deactivate();
    m_pPublishList->deactivate();
    m_pProcessList->deactivate();
    StartTimer(0.5);

};

void  CScopeTabPane::SetMask()
{
    std::map<std::string,ProcessOptions>::iterator p;
    std::set<std::string> Mask;
    for(p = m_ProcessOptions.begin();p!=m_ProcessOptions.end();p++)
    {
        if(!p->second.m_bShow)
            Mask.insert(p->first);
    }

    m_DBImage.Clear();
    m_DBImage.SetMask(Mask);
    ResetSearchAutocompleteState();
    if(m_pScopeGrid)
    {
        const char* pFilter = (m_pSearchInput ? m_pSearchInput->value() : "");
        m_pScopeGrid->SetFilter(pFilter ? pFilter : "");
    }
}


std::string CScopeTabPane::GetFocusProcess()
{
    const char * pStr = m_pProcessList->text(m_pProcessList->value());
    if(pStr==NULL)
        return "";
    return std::string(pStr);
}


void CScopeTabPane::OnControlWidget(Fl_Widget* pWidget,int ID)
{
    //this is the switch yard for all messages
    switch(ID)
    {
    case ID_PROCESS:
        {
            if(Fl::event_shift())
            {
                //toggling visibility masks...
                std::string sFocus = GetFocusProcess();

                if(m_ProcessOptions.find(sFocus)!=m_ProcessOptions.end())
                {
                    ProcessOptions & rOptions = m_ProcessOptions[sFocus];
                    rOptions.m_bShow = !rOptions.m_bShow;
                }

                m_pProcessList->checked(m_pProcessList->value(),m_ProcessOptions[GetFocusProcess()].m_bShow);

                SetMask();

            }
            else 
            {
                //changing what appear isn teh subscribed/published box
                std::string sFocus= GetFocusProcess();                

                STRING_LIST sSubs,sPubs;
                if(m_DBImage.GetProcInfo(sFocus,sSubs,sPubs))
                {
                    m_pSubscribeList->clear();
                    m_pPublishList->clear();
                    STRING_LIST::iterator p;
                    for(p = sSubs.begin();p!=sSubs.end();p++)
                    {
                        m_pSubscribeList->add(p->c_str());
                    }
                    for(p = sPubs.begin();p!=sPubs.end();p++)
                    {
                        m_pPublishList->add(p->c_str());
                    }
                    static char sPubTxt[1024];
                    sprintf(sPubTxt,"%s Publishes",sFocus.c_str());
                    static char sSubTxt[1024];
                    sprintf(sSubTxt,"%s Subscribes",sFocus.c_str());

                }
                m_pProcessList->checked(m_pProcessList->value(),m_ProcessOptions[GetFocusProcess()].m_bShow);
            }
        }

        break;
    case ID_CONNECT:
        {
            std::string sMOOSName = "uMSPlus["+m_Comms.GetLocalIPAddress()+"]";
            m_sHost = std::string(m_pDBHostInput->value());
            m_lPort = atoi(m_pDBPortInput->value());
            //set up callbacks
            m_Comms.SetOnConnectCallBack(MOOSConnectCallback,this);
            m_Comms.SetOnDisconnectCallBack(MOOSDisconnectCallback,this);

            //go!
            if(!m_Comms.Run(m_sHost.c_str(),m_lPort,sMOOSName.c_str()))
            {
            }
        }
        break;

    case ID_SEARCH:
        {
            ResetSearchAutocompleteState();
            if(m_pScopeGrid && m_pSearchInput)
            {
                m_pScopeGrid->SetFilter(m_pSearchInput->value());
            }
        }
        break;

    case ID_SHOW_PENDING:
        {
            Fl_Button* pB = (Fl_Button*)GetByID(ID_SHOW_PENDING);
            m_DBImage.Clear();
            SetMask();
            m_DBImage.ShowPending(pB->value()!=0);
        }
        break;

    }

}

void CScopeTabPane::HandleSearchAutocomplete()
{
    if(m_pSearchInput==NULL)
        return;

    const char* pCurrent = m_pSearchInput->value();
    std::string sCurrent = pCurrent ? pCurrent : "";
    std::string sPrefixUpper = sCurrent;
    MOOSToUpper(sPrefixUpper);

    std::vector<std::string> sMatches;
    int nVars = m_DBImage.GetNumVariables();
    sMatches.reserve(nVars);
    for(int i = 0; i < nVars; ++i)
    {
        CDBImage::CVar Var;
        if(m_DBImage.Get(Var,i))
        {
            std::string sName = Var.GetName();
            if(sPrefixUpper.empty())
            {
                sMatches.push_back(sName);
            }
            else
            {
                std::string sNameUpper = sName;
                MOOSToUpper(sNameUpper);
                if(sNameUpper.find(sPrefixUpper)==0)
                {
                    sMatches.push_back(sName);
                }
            }
        }
    }

    if(sMatches.empty())
    {
        if(sPrefixUpper != m_LastAutocompletePrefix)
        {
            Fl::beep(FL_BEEP_DEFAULT);
        }
        m_LastAutocompletePrefix = sPrefixUpper;
        m_AutocompleteCandidates.clear();
        HideSearchSuggestions();
        return;
    }

    std::sort(sMatches.begin(), sMatches.end(), CaseInsensitiveLess());

    std::string sLongest = sMatches[0];
    for(size_t i = 1; i < sMatches.size(); ++i)
    {
        size_t nLimit = std::min(sLongest.size(), sMatches[i].size());
        size_t j = 0;
        for(; j < nLimit; ++j)
        {
            if(std::toupper((unsigned char)sLongest[j]) != std::toupper((unsigned char)sMatches[i][j]))
                break;
        }
        sLongest = sLongest.substr(0, j);
        if(sLongest.empty())
            break;
    }

    std::string sLongestUpper = sLongest;
    MOOSToUpper(sLongestUpper);

    bool bChanged = false;

    if(!sLongest.empty() && (sLongestUpper.size() > sPrefixUpper.size() || sLongestUpper != sPrefixUpper))
    {
        if(sCurrent != sLongest)
        {
            m_pSearchInput->value(sLongest.c_str());
            m_pSearchInput->position((int)sLongest.size());
            bChanged = true;
        }
        HideSearchSuggestions();
    }
    else if(sMatches.size()==1)
    {
        const std::string& sOnly = sMatches.front();
        if(sCurrent != sOnly)
        {
            m_pSearchInput->value(sOnly.c_str());
            m_pSearchInput->position((int)sOnly.size());
            bChanged = true;
        }
        HideSearchSuggestions();
    }
    else
    {
        ShowSearchSuggestions(sMatches);
    }

    std::string sEffectivePrefix = m_pSearchInput ? m_pSearchInput->value() : "";
    MOOSToUpper(sEffectivePrefix);
    m_LastAutocompletePrefix = sEffectivePrefix;
    m_AutocompleteCandidates = sMatches;

    if(m_pScopeGrid)
    {
        m_pScopeGrid->SetFilter(m_pSearchInput->value());
    }
}

void CScopeTabPane::ResetSearchAutocompleteState()
{
    m_LastAutocompletePrefix.clear();
    m_AutocompleteCandidates.clear();
    HideSearchSuggestions();
}

void CScopeTabPane::HideSearchSuggestions()
{
    if(m_pSearchSuggestionWindow && m_pSearchSuggestionWindow->shown())
    {
        m_pSearchSuggestionWindow->hide();
    }
}

void CScopeTabPane::ShowSearchSuggestions(const std::vector<std::string>& suggestions)
{
    if(suggestions.empty() || m_pSearchInput==NULL)
    {
        HideSearchSuggestions();
        return;
    }

    if(m_pSearchSuggestionWindow==NULL)
    {
        m_pSearchSuggestionWindow = new SearchSuggestionWindow();
        m_pSearchSuggestionWindow->labelsize(m_pSearchInput->textsize());
    }

    std::string sDisplay;
    const size_t nLimit = 30;
    size_t nCount = std::min(nLimit, suggestions.size());
    for(size_t i = 0; i < nCount; ++i)
    {
        sDisplay += suggestions[i];
        if(i+1<nCount)
            sDisplay += "\n";
    }
    if(suggestions.size() > nLimit)
    {
        sDisplay += "\n...";
    }

    m_pSearchSuggestionWindow->value(sDisplay);

    int nX = 0;
    int nY = 0;
    for(Fl_Widget* pWidget = m_pSearchInput; pWidget!=NULL; pWidget = pWidget->parent())
    {
        nX += pWidget->x();
        nY += pWidget->y();
    }
    nY += m_pSearchInput->h();
    m_pSearchSuggestionWindow->position(nX, nY);
    m_pSearchSuggestionWindow->show();
}


bool CScopeTabPane::MOOSConnectCallback(void * pParam)
{
    if(pParam)
    {
        return ((CScopeTabPane*)pParam)->OnMOOSConnect();
    }
    return false;
}

bool CScopeTabPane::MOOSDisconnectCallback(void * pParam)
{
    if(pParam)
    {
        return ((CScopeTabPane*)pParam)->OnMOOSDisconnect();
    }
    return false;
}


bool CScopeTabPane::OnMOOSConnect()
{
    m_pScopeGrid->activate();
    m_pProcessList->activate();
    m_pSubscribeList->activate();
    m_pPublishList->activate();

    m_pConnectButton->deactivate();
    m_pDBPortInput->deactivate();
    m_pDBHostInput->deactivate();

    return true;
}


bool CScopeTabPane::OnMOOSDisconnect()
{

    m_pScopeGrid->deactivate();
    m_pProcessList->deactivate();
    m_pSubscribeList->deactivate();

    m_pPublishList->deactivate();

    m_pConnectButton->activate();
    m_pDBPortInput->activate();
    m_pDBHostInput->activate();

    return true;

}

bool CScopeTabPane::FetchLoop()
{
    while(!m_FetchThread.IsQuitRequested())
    {
        if(m_Comms.IsConnected())
        {
            if(m_nCounts++%5==0)
            {
                if(!GetDBProcSummary())
                {                
                }
            }
            if(!GetDBSummary())
            {
            }
        }
        //basic 4Hz Tick - 
        MOOSPause(250);
    }

    return true;

}




void CScopeTabPane::OnTimer()
{
    Fl_Widget* pActive = ((Fl_Tabs*)parent())->value();
    if(pActive==this)
    {
        m_pScopeGrid->redraw();

        //we'll update the process list much slower...
        if(m_nCounts%5==0)
        {
            STRING_LIST sProcs;
            if(m_DBImage.GetProcesses(sProcs))
            {
                const char * pStr = m_pProcessList->text(m_pProcessList->value());

                std::string sSel = pStr==NULL ? "": std::string(pStr);

                m_pProcessList->clear();
                int nSel = -1;
                STRING_LIST::iterator q;
                for(q = sProcs.begin();q!=sProcs.end();q++)
                {            
                    ProcessOptions & rOptions = m_ProcessOptions[q->c_str()];

                    m_pProcessList->add(q->c_str());
                    nSel= m_pProcessList->nitems();

                    m_pProcessList->checked(nSel,rOptions.m_bShow);

                }
            }
            m_pScopeGrid->SetTitle(MOOSFormat("%d Processes %d Variables",sProcs.size(),
                m_DBImage.GetNumVariables()).c_str());
        }
    }
}

bool CScopeTabPane::GetDBSummary()
{
    if(!m_Comms.IsConnected())
    {
        return true;
    }
    else
    {
        MOOSMSG_LIST InMail;
        if(m_Comms.ServerRequest("ALL",InMail))
        {
            return m_DBImage.Set(InMail);
        }
    }
    return false;
}

bool CScopeTabPane::GetDBProcSummary()
{
    MOOSMSG_LIST InMail;
    if(!m_Comms.IsConnected())
    {
        return true;
    }
    else
    {
        if(m_Comms.ServerRequest("PROC_SUMMARY",InMail))
        {
            if(m_DBImage.SetProcInfo(InMail))
            {
            }
        }
    }

    return false;
}


