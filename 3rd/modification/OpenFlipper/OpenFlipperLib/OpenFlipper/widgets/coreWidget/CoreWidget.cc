/*===========================================================================*\
 *                                                                           *
 *                              OpenFlipper                                  *
 *      Copyright (C) 2001-2010 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openflipper.org                             *
 *                                                                           *
 *---------------------------------------------------------------------------*
 *  This file is part of OpenFlipper.                                        *
 *                                                                           *
 *  OpenFlipper is free software: you can redistribute it and/or modify      *
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenFlipper is distributed in the hope that it will be useful,           *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenFlipper. If not,                                  *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/

/*===========================================================================*\
 *                                                                           *
 *   $Revision: 9595 $                                                         *
 *   $Author: moebius $                                                      *
 *   $Date: 2010-06-17 12:48:23 +0200 (Do, 17. Jun 2010) $                   *
 *                                                                           *
\*===========================================================================*/



//=============================================================================
//
//  CLASS MViewWidget - IMPLEMENTATION
//
//=============================================================================


//== INCLUDES =================================================================

// -------------------- mview
#include "CoreWidget.hh"

// -------------------- OpenFlipper Includes
#include <OpenFlipper/common/GlobalOptions.hh>
#include <OpenFlipper/common/RecentFiles.hh>
#include <OpenFlipper/BasePlugin/PluginFunctions.hh>
#include <OpenFlipper/BasePlugin/PluginFunctionsCore.hh>
#include <OpenFlipper/BasePlugin/KeyInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>

#include <OpenFlipper/widgets/videoCaptureDialog/VideoCaptureDialog.hh>

#include <OpenFlipper/widgets/glWidget/CursorPainter.hh>

#define WIDGET_HEIGHT 800
#define WIDGET_WIDTH  800

//== IMPLEMENTATION ==========================================================

/** \brief Constuctor for the Core Widget
 *
*/
CoreWidget::
CoreWidget( QVector<ViewMode*>& _viewModes,
            std::vector<PluginInfo>& _plugins,
            QList< SlotInfo >& _coreSlots ) :
  QMainWindow(),
  coreSlots_(_coreSlots),
  shiftPressed_(false),
  fullscreenState_(0),
  viewModes_(_viewModes),
  viewModeButton_(0),
  viewModeMenu_(0),
  splitter_(0),
  logWidget_(0),
  recentFilesMenu_(0),
  helpMenu_(0),
  cursorPainter_(0),
  sceneGraphDialog_(0),
  fileMenu_(0),
  viewMenu_(0),
  toolsMenu_(0),
  fileMenuEnd_(0),
  stereoButton_(0),
  moveButton_(0),
  lightButton_(0),
  pickButton_(0),
  questionButton_(0),
  globalDrawMenu_(0),
  drawGroup_(0),
  viewGroup_(0),
  perspectiveProjectionAction_(0),
  activeDrawModes_(0),
  availableGlobalDrawModes_(0),
  contextMenu_(0),
  contextSelectionMenu_(0),
  drawGroupViewer_(0),
  viewerDrawMenu_(0),
  coordSysMenu_(0),
  snapshotName_(""),
  snapshotCounter_(0),
  stackMenu_(0),
  helpWidget_(0),
  stereoSettingsWidget_(0),
  aboutWidget_(0),
  optionsWidget_(0),
  plugins_(_plugins),
  stereoActive_(false),
  actionMode_(Viewer::PickingMode),
  lastActionMode_(Viewer::ExamineMode),
  pickMenu_(0),
  pick_mode_name_(""),
  pick_mode_idx_(-1)
{
  setupStatusBar();

  defaultIconSize_ = iconSize();

  toolSplitter_ = new QSplitter(Qt::Horizontal,this);

  setCentralWidget(toolSplitter_);

  splitter_ = new QSplitter(Qt::Vertical,toolSplitter_);
  stackedWidget_ = new QStackedWidget(splitter_);

  QGLFormat format = QGLFormat::defaultFormat();

  #ifdef ARCH_DARWIN
  format.setStereo(false);
  #else
  format.setStereo( OpenFlipper::Options::stereo() );
  #endif
  format.setAlpha(true);
  format.setStencil(true);
  format.setSampleBuffers(true);
  QGLFormat::setDefaultFormat(format);

  // Construct GL context & widget
  baseLayout_ = new QtMultiViewLayout;
  baseLayout_->setContentsMargins(0,0,0,0);

  // ===============================================================================
  // Test context capabilities ...
  // If we get stereo buffers, we use them .. which might disable multisampling
  // If we dont have stereo, we disable it to not interfere with multisampling
  // ===============================================================================
  QGLWidget* test = new QGLWidget(format);
  if ( ! test->format().stereo() ) {
    //     std::cerr << "No stereo ... disabling stereo for real context!" << std::endl;
    format.setStereo(false);
    QGLFormat::setDefaultFormat(format);
  }/* else {
    std::cerr << "Stereo found ok" << std::endl;
  }*/

  delete test;

  glWidget_ = new QGLWidget(format,0);
  PluginFunctions::shareGLWidget (glWidget_);

  glView_ = new QtGLGraphicsView(stackedWidget_);
  glScene_ = new QtGLGraphicsScene (&examiner_widgets_, baseLayout_);

  // is stereo possible, use it?
  OpenFlipper::Options::glStereo(glWidget_->format().stereo());

  glView_->setViewport(glWidget_);
  glView_->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
  glView_->setScene(glScene_);

  // gl widget as parent to make sure that the CursorPainter will be deleted before
  cursorPainter_ = new CursorPainter (glWidget_);
  cursorPainter_->setForceNative (OpenFlipperSettings().value("Core/Gui/glViewer/nativeMouse",false).toBool() );
  cursorPainter_->setEnabled (stereoActive_);
  glScene_->setCursorPainter (cursorPainter_);

  centerWidget_ = new QGraphicsWidget;
  glScene_->addItem(centerWidget_);
  centerWidget_->setGeometry (glScene_->sceneRect ());

  connect ( glView_, SIGNAL( sceneRectChanged( const QRectF & ) ),
            this, SLOT( sceneRectChanged( const QRectF & ) ) );

  stackedWidget_->addWidget(glView_);
  stackWidgetList_.push_back( StackWidgetInfo( false, "3D Examiner Widget", glView_ ) );

  // ======================================================================
  // Set up the logging window
  // ======================================================================

  slidingLogger_ = new QtSlideWindow (tr("Log Viewer"), centerWidget_);
  tempLogWidget = new QWidget;

  logWidget_ = new LoggerWidget(splitter_);
  logWidget_->setSizePolicy( QSizePolicy ( QSizePolicy::Preferred , QSizePolicy::Preferred ) );
  logWidget_->resize( splitter_->width() ,240);

  originalLoggerSize_ = 0;
  loggerState_ = OpenFlipper::Options::Normal;

  QList<int> wsizes( splitter_->sizes() );

  if (OpenFlipper::Options::loggerState() == OpenFlipper::Options::InScene) {
    slidingLogger_->attachWidget (logWidget_);
    splitter_->insertWidget (1, tempLogWidget);
    wsizes[0] = 1;
    wsizes[1] = 0;
    splitter_->setSizes(wsizes);
    loggerState_ = OpenFlipper::Options::InScene;
    baseLayout_->setContentsMargins (0, 0, 0, 16);
  } else if (OpenFlipper::Options::loggerState() == OpenFlipper::Options::Hidden) {
    splitter_->insertWidget (1, tempLogWidget);
    wsizes[0] = 1;
    wsizes[1] = 0;
    splitter_->setSizes(wsizes);
    loggerState_ = OpenFlipper::Options::Hidden;
  } else {
    // Set initial values to have a usable state
    wsizes[0] = 480;
    wsizes[1] = 240;
    splitter_->setSizes(wsizes);
  }


  // ======================================================================
  // Create examiner
  // ======================================================================

  if ( !OpenFlipperSettings().value("Core/Gui/glViewer/useMultipleViewers",true).toBool() ) {

    glViewer* examinerWidget = new glViewer(glScene_,
					    glWidget_,
                                            PluginFunctions::viewerProperties(0),
					    centerWidget_);

    examiner_widgets_.push_back(examinerWidget);

    examinerWidget->sceneGraph( PluginFunctions::getSceneGraphRootNode() );

    baseLayout_->addItem(examinerWidget, 0);

    cursorPainter_->registerViewer (examinerWidget);

  } else {


    // Create examiners
    for ( unsigned int i = 0 ; i < OpenFlipper::Options::examinerWidgets() ; ++i ) {
      glViewer* newWidget = new glViewer(glScene_,
					 glWidget_,
                                         PluginFunctions::viewerProperties(i),
					 centerWidget_);

      examiner_widgets_.push_back(newWidget);

      connect (&PluginFunctions::viewerProperties(i), SIGNAL( getPickMode(std::string&) ),
               this,                                   SLOT( getPickMode(std::string&) ),Qt::DirectConnection );
      connect (&PluginFunctions::viewerProperties(i), SIGNAL( setPickMode(const std::string) ),
               this,                                   SLOT( setPickMode(const std::string) ),Qt::DirectConnection );
      connect (&PluginFunctions::viewerProperties(i), SIGNAL( getActionMode(Viewer::ActionMode&) ),
               this,                                   SLOT( getActionMode(Viewer::ActionMode&) ),Qt::DirectConnection );
      connect (&PluginFunctions::viewerProperties(i), SIGNAL( setActionMode(const Viewer::ActionMode) ),
               this,                                   SLOT( setActionMode(const Viewer::ActionMode)), Qt::DirectConnection );

    }

    // Initialize all examiners
    for ( unsigned int i = 0 ; i < OpenFlipper::Options::examinerWidgets() ; ++i ) {
      examiner_widgets_[i]->sceneGraph( PluginFunctions::getSceneGraphRootNode() );
      cursorPainter_->registerViewer (examiner_widgets_[i]);
    }

    baseLayout_->addItem(examiner_widgets_[0],0);
    baseLayout_->addItem(examiner_widgets_[1],1);
    baseLayout_->addItem(examiner_widgets_[2],2);
    baseLayout_->addItem(examiner_widgets_[3],3);
  }

  // Make examiner available to the plugins ( defined in PluginFunctions.hh)
  PluginFunctions::setViewers( examiner_widgets_ );

  centerWidget_->setLayout(baseLayout_);


  // ======================================================================
  // Setup dragging for examiner widget
  // ======================================================================
  for ( uint i = 0 ; i < OpenFlipper::Options::examinerWidgets() ; ++i ) {

    connect( examiner_widgets_[i], SIGNAL(startDragEvent( QMouseEvent*)),
             this, SLOT(startDrag(QMouseEvent* )));
    connect( examiner_widgets_[i], SIGNAL(dragEnterEvent( QDragEnterEvent*)),
            this, SLOT(dragEnterEvent(QDragEnterEvent* )));
    connect( examiner_widgets_[i], SIGNAL(dropEvent( QDropEvent*)),
            this, SLOT(dropEvent(QDropEvent* )));
    connect (examiner_widgets_[i], SIGNAL(signalMakeActive ()),
             this, SLOT(slotActivateExaminer()));

    connect (examiner_widgets_[i], SIGNAL(statusMessage(QString, int)),
             statusBar_, SLOT(showMessage(const QString&, int)));
  }


  // ======================================================================
  // Create main Toolbar
  // ======================================================================

  mainToolbar_ = new QToolBar(tr("Main Toolbar"));
  mainToolbar_->setWindowTitle(tr("Main Toolbar"));
  mainToolbar_->setObjectName(tr("MainToolbar"));
  slotAddToolbar(mainToolbar_);

  // ======================================================================
  // Get Toolbar from examiner and integrate it into main window
  // ======================================================================


  // Create the toolbar
  viewerToolbar_ = new QToolBar( tr("Viewer Toolbar" ), this );
  viewerToolbar_->setOrientation(Qt::Vertical);
  viewerToolbar_->setAllowedAreas(Qt::AllToolBarAreas);
  viewerToolbar_->setIconSize(QSize(20,20));
  viewerToolbar_->setObjectName(tr("ViewerToolbar"));

  slotAddToolbar(viewerToolbar_);

  moveButton_ = new QToolButton( viewerToolbar_ );
  moveButton_->setIcon( QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"transform-move.png") );
  moveButton_->setMinimumSize( 16, 16 );
  moveButton_->setMaximumSize( 32, 32 );
  moveButton_->setToolTip( tr("Switch to <b>move</b> mode.") );
  moveButton_->setWhatsThis(tr(
                  "Switch to <b>move</b> mode.<br>"
                  "<ul><li><b>Rotate</b> using <b>left</b> mouse button.</li>"
                  "<li><b>Translate</b> using <b>middle</b> mouse button.</li>"
                  "<li><b>Zoom</b> using <b>left+middle</b> mouse buttons.</li></ul>" ));

  connect( moveButton_,SIGNAL( clicked() ), this, SLOT( setExamineMode() ) );
/*
  connect( this, SIGNAL( actionModeChanged( Viewer::ActionMode ) ),
            this, SLOT(   slotActionModeChanged(Viewer::ActionMode) ) );*/

  viewerToolbar_->addWidget( moveButton_ )->setText(tr("Move"));
  moveButton_->setDown(true);


  lightButton_ = new QToolButton( viewerToolbar_ );
  lightButton_->setIcon( QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"light-mode.png") );
  lightButton_->setMinimumSize( 16, 16 );
  lightButton_->setMaximumSize( 32, 32 );
  lightButton_->setToolTip(tr("Switch to <b>light</b> mode."));
  lightButton_->setWhatsThis(tr(
                  "Switch to <b>light</b> mode.<br>"
                  "Rotate lights using left mouse button."));

  connect( lightButton_,SIGNAL( clicked() ), this, SLOT( setLightMode() ) );
  viewerToolbar_->addWidget( lightButton_)->setText(tr("Light"));


  pickButton_ = new QToolButton( viewerToolbar_ );
  pickButton_->setIcon( QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"pick.png") );
  pickButton_->setMinimumSize( 16, 16 );
  pickButton_->setMaximumSize( 32, 32 );
  pickButton_->setToolTip(tr("Switch to <b>picking</b> mode."));
  pickButton_->setWhatsThis(tr(
                  "Switch to <b>picking</b> mode.<br>"
                  "Use picking functions like flipping edges.<br>"
                  "To change the mode use the right click<br>"
                  "context menu in the viewer."));
  connect( pickButton_,SIGNAL( clicked() ), this, SLOT( setPickingMode() ) );
  viewerToolbar_->addWidget( pickButton_)->setText(tr("Pick"));


  questionButton_ = new QToolButton( viewerToolbar_ );
  questionButton_->setIcon( QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"help-about.png") );
  questionButton_->setMinimumSize( 16, 16 );
  questionButton_->setMaximumSize( 32, 32 );
  questionButton_->setToolTip(tr("Switch to <b>identification</b> mode."));
  questionButton_->setWhatsThis(tr(
                  "Switch to <b>identification</b> mode.<br>"
                  "Use identification mode to get information "
                  "about objects. Click on an object and see "
                  "the log output for information about the "
                  "object."));
  connect( questionButton_,SIGNAL( clicked() ), this, SLOT( setQuestionMode() ) );
  viewerToolbar_->addWidget( questionButton_)->setText(tr("Question"));

  viewerLayoutBox_ = new QComboBox( viewerToolbar_ );
  viewerLayoutBox_->setMinimumSize( 32, 16 );
  viewerLayoutBox_->setMaximumSize( 64, 32 );
  viewerLayoutBox_->setToolTip(tr("Switch <b>viewer layout</b>."));
  viewerLayoutBox_->setWhatsThis(tr(
                  "Switch <b>viewer layout</b>.<br>"
                  "Select the desired viewer layout. "
                  "Possible layouts are: "
                  "<ul> "
				  "<li>Single viewer</li>"
				  "<li>Multiple viewers (grid)</li>"
				  "<li>Multiple viewers (hsplit)</li>"
				  "</ul>"));

  viewerLayoutBox_->addItem(QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"singleviewmode.png"), "");
  viewerLayoutBox_->addItem(QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"multiviewmode1.png"), "");
  viewerLayoutBox_->addItem(QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"multiviewmode2.png"), "");

  viewerLayoutBox_->setIconSize(QSize(22,22));

  connect( viewerLayoutBox_,SIGNAL( activated(int) ), this, SLOT( setViewerLayout(int) ) );

  viewerToolbar_->addWidget( viewerLayoutBox_ );

  viewerToolbar_->addSeparator();

  if (OpenFlipper::Options::stereo())
  {
    stereoButton_ = new QToolButton( viewerToolbar_ );
    stereoButton_->setIcon(  QIcon(OpenFlipper::Options::iconDirStr()+OpenFlipper::Options::dirSeparator()+"mono.png")  );
    stereoButton_->setMinimumSize( 16, 16 );
    stereoButton_->setMaximumSize( 32, 32 );
    stereoButton_->setCheckable( true );
    stereoButton_->setToolTip(tr( "Toggle stereo viewing"));
    // We want a custom context menu
    stereoButton_->setContextMenuPolicy(Qt::CustomContextMenu);
    stereoButton_->setWhatsThis(tr(
                  "Toggle stereo mode<br><br>"
                  "Use this button to switch between stereo "
                  "and mono view. To use this feature you need "
                  "a stereo capable graphics card and a stereo "
                  "display/projection system."));
    connect( stereoButton_, SIGNAL( clicked() ), this , SLOT( slotToggleStereoMode() ) );
    // Custom context menu
    connect( stereoButton_, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(stereoButtonContextMenu(const QPoint &)));
    viewerToolbar_->addWidget( stereoButton_ )->setText( tr("Stereo"));
  }


  addToolBar(Qt::TopToolBarArea,viewerToolbar_);

  // Remember logger size
  wsizes = splitter_->sizes();
  originalLoggerSize_  = wsizes[1];

  // ======================================================================
  // Create ToolBox area
  // ======================================================================

  toolBoxArea_ = new QWidget (toolSplitter_);

  QGroupBox *gb = new QGroupBox (tr("ViewMode"));

  QHBoxLayout *hLayout = new QHBoxLayout;

  vmChangeButton_ = new QPushButton(tr("Change View Mode"));
  QPushButton* vmEditButton   = new QPushButton(tr("Edit View Modes"));

  hLayout->addWidget(vmChangeButton_);
  hLayout->addWidget(vmEditButton);
  gb->setLayout (hLayout);

  connect(vmChangeButton_, SIGNAL(clicked()), this, SLOT(slotViewChangeDialog()));
  connect(vmEditButton, SIGNAL(clicked()), this, SLOT(slotViewModeDialog()));

  toolBoxScroll_ = new QScrollArea ();
  toolBox_ = new SideArea ();
  toolBoxScroll_->setWidget (toolBox_);
  toolBoxScroll_->setWidgetResizable (true);
  toolBoxScroll_->setFrameStyle (QFrame::StyledPanel);

  QVBoxLayout *vLayout = new QVBoxLayout;
  vLayout->addWidget(gb);
  vLayout->addWidget(toolBoxScroll_);

  toolBoxArea_->setLayout (vLayout);

  wsizes = toolSplitter_->sizes();

  wsizes[0] = 480;
  wsizes[1] = 240;
  toolSplitter_->setSizes(wsizes);

  // ======================================================================
  // Create pick ToolBar
  // ======================================================================

  pickToolbar_ = new QtPickToolbar (this, centerWidget_);

  // ======================================================================
  // Context menu setup
  // ======================================================================

  for ( unsigned int i = 0 ; i < OpenFlipper::Options::examinerWidgets() ; ++i ) {
    connect( examiner_widgets_[i] , SIGNAL(signalCustomContextMenuRequested( const QPoint&) ) ,
             this                 , SLOT( slotCustomContextMenu( const QPoint&) ) );
  }

  contextMenu_ = new QMenu(this);
  contextSelectionMenu_ = new QMenu(tr("Selection"),0);

  setupMenuBar();

  statusBar_->showMessage(tr("Ready"), 5000);

  registerCoreKeys();

  setExamineMode();

  setWindowIcon( OpenFlipper::Options::OpenFlipperIcon() );

  // Create stereo settings widget
  stereoSettingsWidget_ = new StereoSettingsWidget(this);
  // Make it look like a dialog
  stereoSettingsWidget_->setWindowFlags(Qt::Popup);
  // Connect combo boxes and sliders to local slots
  connect(stereoSettingsWidget_->stereoOpengl, SIGNAL(clicked()),
          this, SLOT(slotApplyStereoSettings()));
  connect(stereoSettingsWidget_->stereoAnaglyph, SIGNAL(clicked()),
          this, SLOT(slotApplyStereoSettings()));
  connect(stereoSettingsWidget_->stereoCustomAnaglyph, SIGNAL(clicked()),
          this, SLOT(slotApplyStereoSettings()));
  connect(stereoSettingsWidget_->stereoPhilips, SIGNAL(clicked()),
          this, SLOT(slotApplyStereoSettings()));

  connect(stereoSettingsWidget_->focalDistance, SIGNAL(sliderReleased()),
          this, SLOT(slotApplyStereoSettings()));
  connect(stereoSettingsWidget_->eyeDistance, SIGNAL(editingFinished()),
          this, SLOT(slotApplyStereoSettings()));

  connect(stereoSettingsWidget_->headerContentType, SIGNAL(currentIndexChanged(int)),
          this, SLOT(slotApplyStereoSettings(int)));
  connect(stereoSettingsWidget_->headerFactor, SIGNAL(sliderReleased()),
          this, SLOT(slotApplyStereoSettings()));
  connect(stereoSettingsWidget_->headerOffsetCC, SIGNAL(sliderReleased()),
          this, SLOT(slotApplyStereoSettings()));
  connect(stereoSettingsWidget_->headerSelect, SIGNAL(currentIndexChanged(int)),
          this, SLOT(slotApplyStereoSettings(int)));

  // Close button
  connect(stereoSettingsWidget_->closeButton, SIGNAL(clicked()),
          stereoSettingsWidget_, SLOT(hide()));
}


//-----------------------------------------------------------------------------


CoreWidget::~CoreWidget() {

}

//-----------------------------------------------------------------------------

/** Set viewer to Fullscreen Mode and back
  */
void
CoreWidget::toggleFullscreen() {

  switch (fullscreenState_){

    case 0:
      //switch to fullscreen
      setWindowState( windowState() | Qt::WindowFullScreen);
      break;

    case 1:

      //fullscreen without toolbars
      if ( ! (windowState() & Qt::WindowFullScreen) )
        setWindowState( windowState() | Qt::WindowFullScreen);

      //hide plugin toolbars
      for (uint p=0; p < plugins_.size(); p++)
        for ( uint j = 0 ; j < plugins_[p].toolbars.size(); ++j )
          plugins_[p].toolbars[j].second->hide();

      //hide main toolbar
      mainToolbar_->hide();

      //hide viewer toolbar
      viewerToolbar_->hide();

      //hide the menubar
      menuBar()->hide();

      //hide the statusbar
      statusBar()->hide();

      //remove viewer frame
      glView_->setFrameStyle(QFrame::NoFrame);

      break;

    default:
      //disable fullscreen
      if ( windowState() & Qt::WindowFullScreen )
        setWindowState( windowState() ^  Qt::WindowFullScreen);

      //show toolbars
      setViewMode( OpenFlipper::Options::currentViewMode() );

      //show the menubar
      menuBar()->show();

      //show the statusbar
      statusBar()->show();

      //add viewer frame
      glView_->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
  }

  fullscreenState_ = (fullscreenState_ + 1) % 3;
  
  OpenFlipperSettings().setValue("Core/Gui/fullscreen", bool( windowState() & Qt::WindowFullScreen) );

  show();
}

//-----------------------------------------------------------------------------

/** Enable or disable Fullscreen Mode
  */
void
CoreWidget::setFullscreen(bool _state ) {
  if ( _state )
    setWindowState( windowState() | Qt::WindowFullScreen);
  else {
    if ( windowState() & Qt::WindowFullScreen )
      setWindowState( windowState() ^  Qt::WindowFullScreen);
  }

  fullscreenState_ = (uint) _state;

  OpenFlipperSettings().setValue("Core/Gui/fullscreen", bool( windowState() & Qt::WindowFullScreen) );

  show();
}

//-----------------------------------------------------------------------------

/** Hide or show logger
  */
void
CoreWidget::toggleLogger() {

  switch (OpenFlipper::Options::loggerState ())
  {
    case OpenFlipper::Options::InScene:
      OpenFlipper::Options::loggerState(OpenFlipper::Options::Normal);
      break;
    case OpenFlipper::Options::Normal:
      OpenFlipper::Options::loggerState(OpenFlipper::Options::Hidden);
      break;
    case OpenFlipper::Options::Hidden:
      OpenFlipper::Options::loggerState(OpenFlipper::Options::InScene);
      break;
  }

  // Hide/Show Logger
  showLogger( OpenFlipper::Options::loggerState() );
}

/** Hide or show logger
  */
void
CoreWidget::showLogger(OpenFlipper::Options::LoggerState _state) {
  //Hide Logger
  if (_state == loggerState_)
    return;

  qreal left, top, right, bottom;
  baseLayout_->getContentsMargins (&left, &top, &right, &bottom);

  switch (_state)
  {
    case OpenFlipper::Options::InScene:
      {
        QList<int> wsizes( splitter_->sizes() );

        // Remember old size
        if (loggerState_ == OpenFlipper::Options::Normal)
          originalLoggerSize_  = wsizes[1];

	if ( originalLoggerSize_ == 0)
          originalLoggerSize_ = 240;

        splitter_->insertWidget (1, tempLogWidget);
        wsizes[0] = wsizes[0]+wsizes[1];
        wsizes[1] = 0;
        splitter_->setSizes(wsizes);
        logWidget_->resize (logWidget_->width (), originalLoggerSize_);
        slidingLogger_->attachWidget (logWidget_);
        baseLayout_->setContentsMargins (left, top, right, 16);
      }
      break;
    case OpenFlipper::Options::Normal:
      {
        if ( originalLoggerSize_ == 0)
          originalLoggerSize_ = 240;

        QList<int> wsizes( splitter_->sizes() );

        if (loggerState_ == OpenFlipper::Options::InScene)
          originalLoggerSize_ = logWidget_->height ();

        slidingLogger_->detachWidget ();
        splitter_->insertWidget (1, logWidget_);
        logWidget_->show ();

        wsizes[0] = wsizes[0]+wsizes[1] - originalLoggerSize_;
        wsizes[1] = originalLoggerSize_;
        splitter_->setSizes(wsizes);
        baseLayout_->setContentsMargins (left, top, right, 0);
      }
      break;
    case OpenFlipper::Options::Hidden:
      {
        QList<int> wsizes( splitter_->sizes() );

        // Remember old size
        if (loggerState_ == OpenFlipper::Options::Normal)
          originalLoggerSize_  = wsizes[1];

	if (loggerState_ == OpenFlipper::Options::InScene)
	{
	  slidingLogger_->detachWidget ();
	  originalLoggerSize_ = logWidget_->height ();
	}

        splitter_->insertWidget (1, tempLogWidget);
        wsizes[0] = wsizes[0]+wsizes[1];
        wsizes[1] = 0;
        splitter_->setSizes(wsizes);
        baseLayout_->setContentsMargins (left, top, right, 0);
      }
      break;
  }
  loggerState_ = _state;

/*
  if ( !_state ) {
    QList<int> wsizes( splitter_->sizes() );

    // Remember old size
    originalLoggerSize_  = wsizes[1];

    int height = logWidget_->height ();

    splitter_->insertWidget (1, tempLogWidget);
    wsizes[0] = wsizes[0]+wsizes[1];
    wsizes[1] = 0;
    splitter_->setSizes(wsizes);
    logWidget_->resize (logWidget_->width (), height);
    slidingLogger_->attachWidget (logWidget_);

  } else if (splitter_->widget (1) == logWidget_) {

    if ( originalLoggerSize_ == 0)
        originalLoggerSize_ = 240;

    QList<int> wsizes( splitter_->sizes() );

    if (wsizes[0] == 0)
      wsizes[0] = height();

    wsizes[0] = wsizes[0]+wsizes[1] - originalLoggerSize_;
    wsizes[1] = originalLoggerSize_;
    splitter_->setSizes(wsizes);
  } else {

    QList<int> wsizes( splitter_->sizes() );

    int height = logWidget_->height ();

    slidingLogger_->detachWidget ();
    splitter_->insertWidget (1, logWidget_);

    wsizes[0] = wsizes[0]+wsizes[1] - height;
    wsizes[1] = height;
    splitter_->setSizes(wsizes);
  }
  */
}

//-----------------------------------------------------------------------------

/** Hide or show toolbox
  */
void
CoreWidget::toggleToolbox() {

  //toggle
  showToolbox( OpenFlipperSettings().value("Core/Gui/ToolBoxes/hidden",false).toBool() );
}

//-----------------------------------------------------------------------------

/** Hide or show toolbox
  */
void
CoreWidget::showToolbox( bool _state ) {

  //toggle
  OpenFlipperSettings().setValue("Core/Gui/ToolBoxes/hidden",!_state);

  if ( OpenFlipperSettings().value("Core/Gui/ToolBoxes/hidden",false).toBool() ){

    //hide ViewMode Selection Widget
    toolBoxArea_->setVisible(false);

  }else{
    //reset last ViewMode
    if (OpenFlipper::Options::currentViewMode().trimmed() == "")
      setViewMode("All");
    else
      setViewMode( OpenFlipper::Options::currentViewMode() );
    toolBoxArea_->setVisible(true);
  }
}

//=============================================================================

void
CoreWidget::addRecent(QString _filename, DataType _type)
{
  //dont add objects to recentMenu while loadind Settings
  if ( OpenFlipper::Options::loadingSettings() ) return;

  OpenFlipper::Options::addRecentFile(_filename, _type);

  updateRecent();

}

//=============================================================================

void
CoreWidget::updateRecent()
{
  if ( recentFilesMenu_ == NULL)
    return;

  recentFilesMenu_->clear();

  QStringList recentFiles = OpenFlipperSettings().value("Core/File/RecentFiles").toStringList();
  QStringList recentTypes = OpenFlipperSettings().value("Core/File/RecentTypes").toStringList();

  for (int i = 0 ; i < recentFiles.size() ; ++i ) {

    QFileInfo fi(recentFiles[i]);

    if (fi.suffix() == "ini")
      recentFilesMenu_->addAction(QIcon(OpenFlipper::Options::iconDirStr() + OpenFlipper::Options::dirSeparator()+"Settings-Icon.png"), recentFiles[i]);
    else
      recentFilesMenu_->addAction(typeIcon( typeId(recentTypes[i]) ), recentFiles[i]);

  }

}

void
CoreWidget::closeEvent ( QCloseEvent * /* event */ ) {
  emit exit();
}

void CoreWidget::showOptionsWidget() {

  if ( OpenFlipper::Options::nogui() )
    return;

  if ( optionsWidget_ == 0 ) {
    optionsWidget_ = new OptionsWidget(plugins_, coreKeys_, invKeys_, 0);
    connect(optionsWidget_,SIGNAL(applyOptions()),this,SIGNAL(applyOptions()));
    connect(optionsWidget_,SIGNAL(saveOptions()),this,SIGNAL(saveOptions()));
    connect(optionsWidget_,SIGNAL(addKeyMapping(int,Qt::KeyboardModifiers,QObject*,int)),
            this,          SLOT(slotAddKeyMapping(int,Qt::KeyboardModifiers,QObject*,int)));

    optionsWidget_->setWindowIcon( OpenFlipper::Options::OpenFlipperIcon() );
  }

  //show the optionsWidget centered
  QPoint center;
  center.setX( x() + width() / 2 );
  center.setY( y() + height() / 2 );

  optionsWidget_->setGeometry(center.x() - optionsWidget_->width() / 2,
                              center.y() - optionsWidget_->height()/ 2, optionsWidget_->width(), optionsWidget_->height());

  optionsWidget_->show();

}

void CoreWidget::nextViewerLayout() {

  if ( OpenFlipperSettings().value("Core/Gui/glViewer/useMultipleViewers",true).toBool() ) {

    switch (baseLayout_->mode()) {
      case QtMultiViewLayout::SingleView:
              baseLayout_->setPrimary (PluginFunctions::activeExaminer ());
              baseLayout_->setMode(QtMultiViewLayout::Grid);

              // Update combo box in the toolbar
              viewerLayoutBox_->setCurrentIndex(1);
              break;
      case QtMultiViewLayout::Grid:
              baseLayout_->setPrimary (PluginFunctions::activeExaminer ());
              baseLayout_->setMode(QtMultiViewLayout::HSplit);

              // Update combo box in the toolbar
              viewerLayoutBox_->setCurrentIndex(2);
              break;
      case QtMultiViewLayout::HSplit:
              baseLayout_->setPrimary (PluginFunctions::activeExaminer ());
              baseLayout_->setMode(QtMultiViewLayout::SingleView);

              // Update combo box in the toolbar
              viewerLayoutBox_->setCurrentIndex(0);
              break;
      }
  }
}


void
CoreWidget::setViewerLayout(int _idx) {

  if ( OpenFlipperSettings().value("Core/Gui/glViewer/useMultipleViewers",true).toBool() ) {

  switch (_idx) {
    case 0:
            baseLayout_->setPrimary (PluginFunctions::activeExaminer ());
            baseLayout_->setMode(QtMultiViewLayout::SingleView);
            break;
    case 1:
            baseLayout_->setPrimary (PluginFunctions::activeExaminer ());
            baseLayout_->setMode(QtMultiViewLayout::Grid);
            break;
    case 2:
            baseLayout_->setPrimary (PluginFunctions::activeExaminer ());
            baseLayout_->setMode(QtMultiViewLayout::HSplit);
            break;
  }

    viewerLayoutBox_->setCurrentIndex(_idx);
  }
}

void
CoreWidget::slotShowSceneGraphDialog()
{
  if ( PluginFunctions::getSceneGraphRootNode() )
  {
    if (!sceneGraphDialog_)
    {
      sceneGraphDialog_ = new ACG::QtWidgets::QtSceneGraphDialog( this, PluginFunctions::getSceneGraphRootNode() );

      for ( unsigned int i = 0 ; i < OpenFlipper::Options::examinerWidgets() ; ++i ) {
        connect(examiner_widgets_[i], SIGNAL(signalSceneGraphChanged(ACG::SceneGraph::BaseNode*)),
                sceneGraphDialog_,    SLOT(update(ACG::SceneGraph::BaseNode*)));

        connect(sceneGraphDialog_, SIGNAL(signalNodeChanged(ACG::SceneGraph::BaseNode*)),
                examiner_widgets_[i], SLOT(updateGL()));
      }

    }

    sceneGraphDialog_->setGeneratorMap(PluginFunctions::getSceneGraphGeneratorList() );
    sceneGraphDialog_->show();
  }
}

//-----------------------------------------------------------------------------

void
CoreWidget::sceneRectChanged(const QRectF &rect)
{
  centerWidget_->setGeometry (rect);
  slidingLogger_->updateGeometry ();
  pickToolbar_->updateGeometry ();
}

//-----------------------------------------------------------------------------

void
CoreWidget::startVideoCaptureDialog(){

  VideoCaptureDialog* dialog = new VideoCaptureDialog();
  dialog->setModal(false);

  connect(dialog, SIGNAL(startVideoCapture(QString,int,bool)), this, SIGNAL(startVideoCapture(QString,int,bool)) );
  connect(dialog, SIGNAL(resizeViewers(int,int)), this, SIGNAL(resizeViewers(int,int)) );
  connect(dialog, SIGNAL(resizeApplication(int,int)), this, SIGNAL(resizeApplication(int,int)) );

  dialog->show();
}

//-----------------------------------------------------------------------------

void CoreWidget::slotActivateExaminer()
{
  glViewer* examiner = dynamic_cast<glViewer*>(QObject::sender());

  if (!examiner)
    return;

  for ( uint i = 0 ; i < OpenFlipper::Options::examinerWidgets() ; ++i ) {

    if (examiner == examiner_widgets_[i])
    {
      PluginFunctions::setActiveExaminer (i);
      break;
    }
  }
}

//-----------------------------------------------------------------------------

/// Use native or gl painted cursor
void CoreWidget::setForceNativeCursor ( bool _state )
{
  cursorPainter_->setForceNative (_state);
}

//=============================================================================

