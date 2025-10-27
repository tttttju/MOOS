/*****************************************************************/
/*    NAME: Michael Benjamin                                     */
/*    ORGN: Dept of Mechanical Engineering, MIT, Cambridge MA    */
/*    FILE: REPLAY_GUI.h                                         */
/*    DATE: May 31st, 2005                                       */
/*                                                               */
/* This file is part of MOOS-IvP                                 */
/*                                                               */
/* MOOS-IvP is free software: you can redistribute it and/or     */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation, either version  */
/* 3 of the License, or (at your option) any later version.      */
/*                                                               */
/* MOOS-IvP is distributed in the hope that it will be useful,   */
/* but WITHOUT ANY WARRANTY; without even the implied warranty   */
/* of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See  */
/* the GNU General Public License for more details.              */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with MOOS-IvP.  If not, see                     */
/* <http://www.gnu.org/licenses/>.                               */
/*****************************************************************/

#ifndef REPLAY_GUI_HEADER
#define REPLAY_GUI_HEADER

#include <string>
#include <list>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include "NavPlotViewer.h"
#include "LogPlotViewer.h"
#include "MY_Repeat_Button.h"
#include "MBTimer.h"
#include "MarineVehiGUI.h"
#include "GUI_IPF.h"
#include "GUI_HelmScope.h"
#include "GUI_VarScope.h"
#include "GUI_AppLogScope.h"
#include "GUI_Encounters.h"
#include "GUI_TaskDiary.h"
#include "ALogDataBroker.h"

//===============================================================
//  REPLAY_GUI
//  该类封装了 uReplay 图形界面所需的全部控件与回调逻辑，
//  负责：
//    1. 初始化主导航视图与日志曲线视图。
//    2. 基于 ALogDataBroker 构建变量、行为、任务等菜单。
//    3. 处理键盘与菜单事件，驱动时间轴步进及自动回放。
//    4. 同步刷新所有子窗口（HelmScope、VarScope、任务日志等）。
//  通过继承 MarineVehiGUI，使回放界面具备标准的海上载具
//  可视化能力，同时扩展出特有的日志浏览组件。
//===============================================================
class REPLAY_GUI : public MarineVehiGUI {
 public:
  /// 构造函数：创建所有子控件并建立默认布局。
  REPLAY_GUI(int w, int h, const char *l=0);
  ~REPLAY_GUI();

  /// 重载窗口尺寸调整逻辑，确保子控件随窗口缩放。
  void   resize(int, int, int, int);
  /// 统一处理键盘、鼠标事件，转换为具体的导航或回放操作。
  int    handle(int);

  /// 设置数据代理，供各视图统一读取日志数据。
  void   setDataBroker(const ALogDataBroker&);
  /// 根据数据代理生成左/右日志曲线的选择菜单。
  void   setLogPlotMenus();
  /// 生成变量历史窗口的筛选菜单。
  void   setVarHistMenus();
  /// 生成应用日志窗口的筛选菜单。
  void   setAppLogMenus();
  /// 生成任务日记窗口的筛选菜单。
  void   setTaskDiaryMenus();
  /// 记录行为与变量的映射，供 IPF 视图快速查找。
  void   setBehaviorVarMap(std::map<std::string, std::string>);
  /// 设定左侧日志曲线初始选择项（平台 + 变量名）。
  void   initLogPlotChoiceA(std::string vname, std::string varname);
  /// 设定右侧日志曲线初始选择项（平台 + 变量名）。
  void   initLogPlotChoiceB(std::string vname, std::string varname);

  /// 初始化多目标函数（IvPFunction）相关菜单。
  void   setIPFPlotMenus();
  /// 初始化相遇分析窗口的菜单。
  void   setEncounterPlotMenus();
  /// 初始化 HelmScope（决策器状态）窗口的菜单。
  void   setHelmPlotMenus();

  /// 在自动播放模式下按当前速度条件性推进时间。
  void   conditionalStep();
  /// 根据当前鼠标位置刷新状态栏显示。
  void   updateXY();

  /// 判断鼠标是否位于导航视图内（用于热点判断）。
  bool   inNavPlotViewer() const;
  /// 判断鼠标是否位于日志视图内。
  bool   inLogPlotViewer() const;

  /// 控制单步播放的时间间隔。
  void   steptime(int val=0);
  /// 切换自动流式播放（启用/暂停/停止）。
  void   streaming(int val=0);
  /// 调整自动播放速度（加速或减速）。
  void   streamspeed(bool faster);

  /// 预设窗口布局（常规、单视图等）。
  void   setWindowLayout(std::string layout="normal");
  /// 设置当前全局回放时间，并刷新所有关联控件。
  void   setCurrTime(double=-1);

  /// 统一调整各控件的位置与尺寸。
  void   resizeWidgetsShape();

  /// 设置左侧日志曲线的文本过滤关键字。
  void   setGrepStr1(std::string s) {m_grep1=s;}
  /// 设置右侧日志曲线的文本过滤关键字。
  void   setGrepStr2(std::string s) {m_grep2=s;}

 protected:
  /// 在主菜单中追加回放相关条目及快捷键绑定。
  void   augmentMenu();

  /// 创建并初始化所有窗口控件。
  void   initWidgets();
  /// 调整控件字体大小，适配不同分辨率。
  void   resizeWidgetsText(int);
  /// 根据当前 warp 值刷新倍率提示信息。
  void   updatePlayRateMsg();
  /// 刷新时间轴子控件（时间显示、按钮状态等）。
  void   updateTimeSubGUI();
  /// 切换左侧日志曲线窗口的显示状态。
  void   toggleLeftLogPlot();
  /// 切换右侧日志曲线窗口的显示状态。
  void   toggleRightLogPlot();

 private:
  // Variants of the base class functions. Invoked in REPLAY_GUI::handle
  /// 左右方向键处理：在不同视图之间切换或移动时间轴。
  void   handleLeftRight(int);
  /// 上下方向键处理：调节时间步长或变量选择。
  void   handleUpDown(int);
  /// 鼠标滚轮或快捷键触发的缩放动作。
  void   zoom(int);

  // Implement these base class virtual functions so that the base
  // class version is not called. We handle all these keys in this class.
  void   cb_HandleLeftRight_i(int) {}
  void   cb_HandleUpDown_i(int) {}
  void   cb_Zoom_i(int) {}

  inline void cb_JumpTime_i(int);
  static void cb_JumpTime(Fl_Widget*, int);

  inline void cb_Step_i(int millisecs);
  static void cb_Step(Fl_Widget*, int millisecs);

  inline void cb_StepType_i(int);
  static void cb_StepType(Fl_Widget*, int);

  inline void cb_LeftLogPlot_i(int);
  static void cb_LeftLogPlot(Fl_Widget*, int);

  inline void cb_RightLogPlot_i(int);
  static void cb_RightLogPlot(Fl_Widget*, int);

  inline void cb_VarHist_i(int);
  static void cb_VarHist(Fl_Widget*, int);

  inline void cb_AppLog_i(int);
  static void cb_AppLog(Fl_Widget*, int);

  inline void cb_TaskDiary_i();
  static void cb_TaskDiary(Fl_Widget*);

  inline void cb_Encounter_i(int);
  static void cb_Encounter(Fl_Widget*, int);

  inline void cb_IPF_GUI_i(int);
  static void cb_IPF_GUI(Fl_Widget*, int);

  inline void cb_Helm_GUI_i(int);
  static void cb_Helm_GUI(Fl_Widget*, int);

  inline void cb_Streaming_i(int);
  static void cb_Streaming(Fl_Widget*, int);

  inline void cb_StreamStep_i(int);
  static void cb_StreamStep(Fl_Widget*, int);

  inline void cb_StreamSpeed_i(bool);
  static void cb_StreamSpeed(Fl_Widget*, bool);

  inline void cb_TimeZoom_i(int);
  static void cb_TimeZoom(Fl_Widget*, int);

  inline void cb_ToggleSyncScales_i(int);
  static void cb_ToggleSyncScales(Fl_Widget*, int);

  inline void cb_ButtonHideLogPlot_i(int);
  static void cb_ButtonHideLogPlot(Fl_Widget*, int);

 public:
  NavPlotViewer *np_viewer;
  LogPlotViewer *lp_viewer;
  
  // 记录已打开的各类子窗口，便于统一刷新和关闭。
  std::list<GUI_IPF*>         m_sub_guis;
  std::list<GUI_HelmScope*>   m_sub_guis_h;
  std::list<GUI_VarScope*>    m_sub_guis_v;
  std::list<GUI_Encounters*>  m_sub_guis_e;
  std::list<GUI_AppLogScope*> m_sub_guis_a;
  std::list<GUI_TaskDiary*>   m_sub_guis_t;

 protected:
  /// 数据访问代理，负责向各视图提供统一的日志数据入口。
  ALogDataBroker m_dbroker;

  int m_left_mix;
  int m_right_mix;

 protected:
  /// 导航视图与日志视图的当前高度缓存，用于布局自适应。
  double      m_np_viewer_hgt;
  double      m_lp_viewer_hgt;

  // map from behavior to scope vars for convenience in GUI_IPF
  /// 行为名称与变量名的映射，协助 IPF 视图自动选择对应变量。
  std::map<std::string, std::string> m_map_bhv_vars;

  /// 顶部时间显示组件。
  Fl_Output*  m_disp_time;

  /// 控制是否显示左/右日志曲线窗口的复选框。
  Fl_Check_Button *m_but_hide_lp;
  Fl_Check_Button *m_but_hide_rp;

  /// 左右日志曲线的变量标签与当前值显示组件。
  Fl_Output*  m_label1;
  Fl_Output*  m_curr1;
  Fl_Output*  m_label2;
  Fl_Output*  m_curr2;

  /// 时间轴缩放与同步相关按钮。
  Fl_Repeat_Button *m_but_zoom_in_time;
  Fl_Repeat_Button *m_but_zoom_out_time;
  Fl_Button        *m_but_zoom_reset_time;
  Fl_Button        *m_but_sync_scales;

 protected:
  /// 是否处于自动流式播放状态。
  bool    m_stream;
  /// 自动播放计时器，用于控制帧推进频率。
  MBTimer m_timer;

  std::string m_replay_warp_msg;
  int         m_replay_warp_ix;
  double      m_replay_warp;
  int         m_replay_disp_gap;  // milliseconds

  std::list<double> m_warp_gaps;
  double            m_replay_warp_actual;

  std::string m_grep1;
  std::string m_grep2;
};
#endif





