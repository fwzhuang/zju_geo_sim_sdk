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
 *   $Revision: 9595 $                                                       *
 *   $Author: moebius $                                                      *
 *   $Date: 2010-06-17 12:48:23 +0200 (Do, 17. Jun 2010) $                   *
 *                                                                           *
\*===========================================================================*/

//=============================================================================
//
// CLASS InfoPlugin
//
// Author:  David Bommes <bommes@cs.rwth-aachen.de>
//=============================================================================

#ifndef INFOPLUGIN_HH
#define INFOPLUGIN_HH

//== INCLUDES =================================================================

#include <QObject>

#include <OpenFlipper/BasePlugin/BaseInterface.hh>
#include <OpenFlipper/BasePlugin/MouseInterface.hh>

#include <OpenFlipper/BasePlugin/StatusbarInterface.hh>
#include <OpenFlipper/BasePlugin/LoggingInterface.hh>
#include <OpenFlipper/common/Types.hh>
#include <ObjectTypes/PolyMesh/PolyMesh.hh>
#include <ObjectTypes/TriangleMesh/TriangleMesh.hh>

#include "infoDialog.hh"
#include "infoBar.hh"

//== CLASS DEFINITION =========================================================


/** Plugin for Info Support
 */
class InfoPlugin : public QObject, BaseInterface, MouseInterface, LoggingInterface, StatusbarInterface
{
  Q_OBJECT
      Q_INTERFACES(BaseInterface)
      Q_INTERFACES(MouseInterface)
      Q_INTERFACES(LoggingInterface)
      Q_INTERFACES(StatusbarInterface)


  signals:
    // BaseInterface
    void setSlotDescription(QString     _slotName,   QString     _slotDescription,
                            QStringList _parameters, QStringList _descriptions);

    // LoggingInterface
    void log(Logtype _type, QString _message);
    void log(QString _message);

    // StatusbarInterface
    void addWidgetToStatusbar(QWidget* _widget);
    
  private slots :
    // BaseInterface
    void initializePlugin();
    void pluginsInitialized();
    void slotObjectUpdated( int _identifier, const UpdateType _type );
    void slotObjectSelectionChanged( int _identifier );
    void slotAllCleared();
    
    // MouseInterface
    void slotMouseEventIdentify( QMouseEvent* _event );

    
    
  public :

  // default constructor
      InfoPlugin(): info_(0),infoBar_(0) {};

  // default destructor
      ~InfoPlugin() {};

      /// Name of the Plugin
      QString name(){ return (QString("Info")); };

      /// Description of the Plugin
      QString description() { return (QString(tr("Providing Information about Objects"))); };

  private :
    InfoDialog* info_;

    InfoBar* infoBar_;
    
    template< class MeshT >
    void printMeshInfo( MeshT* _mesh, int _id, unsigned int _face, ACG::Vec3d& _hitPoint );


//===========================================================================
/** @name Scripting Functions
  * @{ */
//===========================================================================

  public slots:

    /// get total number of vertices for a given object
    int vertexCount(int _id);
    /// get total number of edges for a given object
    int edgeCount(int _id);
    /// get total number of faces for a given object
    int faceCount(int _id);
    /// get the number of boundaries for a given object
    int boundaryCount(int _id);
    /// get the number of components for a given object
    int componentCount(int _id);

    /// get the genus of the given object
    int genus(int _id);
    /// get the center of gravity
    Vector cog(int _id);
    /// get minumum bounding box point
    Vector boundingBoxMin(int _id);
    /// get maximum bounding box point
    Vector boundingBoxMax(int _id);
    /// get the size of the bounding box
    Vector boundingBoxSize(int _id);

    /// get the length of an edge
    double edgeLength(int _id, int _edgeHandle);
    /// get the area of a face
    double faceArea  (int _id, int _faceHandle);
    /// get the aspect ratio of a face
    double aspectRatio(int _id, int _faceHandle);
    /// get vertex valence
    int vertexValence  (int _id, int _vertexHandle);

    /// get the minimal edge length
    double minEdgeLength(int _id);
    /// get the maximal edge length
    double maxEdgeLength(int _id);
    /// get the mean edge length
    double meanEdgeLength(int _id);

  public slots:
    QString version() { return QString("1.0"); };

  private:
    /// set scripting slot descriptions
    void setDescriptions();
    
    /// Get closest vertex index
    template< class MeshT >
    int getClosestVertex(MeshT* _mesh, int _face_idx);
    
    /// Get closest edge index
    template< class MeshT >
    int getClosestEdge(MeshT* _mesh, int _face_idx);

    /// Get edge lengths
    template< class MeshT >
    void getEdgeLengths(MeshT* _mesh, double &min, double &max, double &mean);

    /// Get edge lengths
    bool getEdgeLengths(int _id, double &min, double &max, double &mean);
    
    /// Mouse click hit point on face
    ACG::Vec3d hit_point_;
    
/** @} */

};

#endif //MOVEPLUGIN_HH

