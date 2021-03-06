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
//  Logging System
//
//=============================================================================

#ifndef LOGGING_HH
#define LOGGING_HH

//=============================================================== INCLUDES ====

#include <iostream>

#include <QObject>
#include "OpenFlipper/BasePlugin/LoggingInterface.hh"

//======================================================= CLASS DEFINITION ====
/**
 * \file Logging.hh 
 * This File contains the header for the log wrapper used for Plugins
*/


/** A wrapper class, getting events from an plugin, prepending plugin name to the log
 * and passing it to the core. This class is generated for every plugin of the core.
 * It also does some cleanup of the passed strings.\n
 */
class PluginLogger : public QObject {
  
  Q_OBJECT
  
  public:
    
    /// Standard Constructor 
    PluginLogger(QString _pluginName , Logtype _type = LOGOUT);
    
    /// Destructor
    ~PluginLogger();
  
    /// Provide operator for streaming 
    void operator<< ( const std::string& _s ); 
    
  private slots:
    
    /// Log messages with a given Logtype
    void slotLog(Logtype _type, QString _message);
    
    /// Wrapper which logs at LOGOUT Logtype by default
    void slotLog(QString _message);
    
  signals:
    /// Sends the generated logs to the core
    void log(Logtype , QString);
    
  private:
    /// prepend this name to all output messages
    QString pluginName_;
    
    /// Default logtype ( used for streams )
    Logtype defaultLogType_;
    
};


//=============================================================================
#endif // LOGGING_HH
//=============================================================================
