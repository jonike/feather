/***********************************************************************
 *
 * Filename: sg_editor.cpp
 *
 * Description: Editor widget for scenegraph.
 *
 * Copyright (C) 2015 Richard Layman, rlayman2000@yahoo.com 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ***********************************************************************/

#include "sg_editor.hpp"
#include "commands.hpp"
#include "selection.hpp"
#include "field.hpp"

#define BACKGROUND_COLOR "#696969"
#define NODE_TEXT_COLOR "#000000"
#define SELECTED_NODE_COLOR "#FF007F"
#define DESELECTED_NODE_COLOR "#6A5ACD"
#define HOVER_NODE_COLOR "#FF8C00"
#define SELECTED_IN_CONNECTOR_COLOR "" // TODO
#define DESELECTED_IN_CONNECTOR_COLOR "#50C878"
#define HOVER_CONNECTOR_COLOR "#FFBF00"
#define SELECTED_OUT_CONNECTOR_COLOR "" // TODO
#define DESELECTED_OUT_CONNECTOR_COLOR "#9400D3"
#define SELECTED_CONNECTION_COLOR "#FFEF00"
#define DESELECTED_CONNECTION_COLOR "#99BADD"
#define HOVER_CONNECTION_COLOR ""  // TODO


int MouseInfo::clickX=0;
int MouseInfo::clickY=0;

SGState::Mode SGState::mode=SGState::Normal;
int SGState::srcUid=0;
int SGState::srcNid=0;
int SGState::srcFid=0;
int SGState::tgtUid=0;
int SGState::tgtNid=0;
int SGState::tgtFid=0;
SceneGraphEditor* SGState::pSge=NULL;


// SCENEGRAPH

SceneGraphConnection::SceneGraphConnection(SceneGraphConnection::Connection type, QQuickItem* parent) :
    QQuickPaintedItem(parent),
    m_type(type)
{
    setWidth(CONNECTION_WIDTH);
    setHeight(CONNECTION_HEIGHT);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);

    //int nedges = feather::qml::command::get_node_connection_count(0);
    std::vector<int> edges;
    feather::qml::command::get_node_out_connections(0,edges);
    //feather::FNodeDescriptor n = 0;
    //feather::FFieldConnection connection = boost::edge(n);
    //std::cout << "EDGE for node 0 is " << feather::sg[connection.first].n1 << "=>" << feather::sg[connection.first].n2 << std::endl;
    if(m_type == In)
        m_connFillBrush = QBrush(QColor(DESELECTED_IN_CONNECTOR_COLOR));
    else
        m_connFillBrush = QBrush(QColor(DESELECTED_OUT_CONNECTOR_COLOR));
}

SceneGraphConnection::~SceneGraphConnection()
{
}

void SceneGraphConnection::paint(QPainter* painter)
{
    painter->setRenderHints(QPainter::Antialiasing, true);
    painter->setBrush(m_connFillBrush);
    painter->drawEllipse(QPoint(5,5),CONNECTION_WIDTH/2,CONNECTION_HEIGHT/2);
}

void SceneGraphConnection::mousePressEvent(QMouseEvent* event)
{
    MouseInfo::clickX = event->windowPos().x();
    MouseInfo::clickY = event->windowPos().y();
    SGState::mode = SGState::FieldConnection;
    connClicked(event->button(),m_type);
}

void SceneGraphConnection::mouseReleaseEvent(QMouseEvent* event)
{
    SGState::mode = SGState::Normal;
    update();
}

void SceneGraphConnection::hoverEnterEvent(QHoverEvent* event)
{
    m_connFillBrush.setColor(QColor(HOVER_CONNECTOR_COLOR));
    update();
}

void SceneGraphConnection::hoverLeaveEvent(QHoverEvent* event)
{
    if(m_type == In)
        m_connFillBrush = QBrush(QColor(DESELECTED_IN_CONNECTOR_COLOR));
    else
        m_connFillBrush = QBrush(QColor(DESELECTED_OUT_CONNECTOR_COLOR));
    update();
}

void SceneGraphConnection::mouseMoveEvent(QMouseEvent* event)
{
    std::cout << "connection mouse move event\n";
    MouseInfo::clickX = event->windowPos().x(); 
    MouseInfo::clickY = event->windowPos().y(); 
    SGState::pSge->update();
}


// Node
SceneGraphNode::SceneGraphNode(int _uid, int _node, QQuickItem* parent) : 
    QQuickPaintedItem(parent),
    m_uid(_uid),
    m_node(_node),
    m_x(0),
    m_y(0),
    m_imgDir("ui/icons/"),
    m_nodeFillBrush(QBrush(QColor(DESELECTED_NODE_COLOR))),
    m_pInConn(new SceneGraphConnection(SceneGraphConnection::In,this)),
    m_pOutConn(new SceneGraphConnection(SceneGraphConnection::Out,this))
{
    if(feather::smg::Instance()->selected(m_uid)){
        m_nodeFillBrush.setColor(QColor(SELECTED_NODE_COLOR));
        update();
    }
    setWidth(NODE_WIDTH+4);
    setHeight(NODE_HEIGHT+4);
    m_pOutConn->setX(NODE_WIDTH-2);
    m_pOutConn->setY((NODE_HEIGHT/2)-2);
    m_pInConn->setX(-2);
    m_pInConn->setY((NODE_HEIGHT/2)-2);
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(true);

    connect(m_pInConn,SIGNAL(connClicked(Qt::MouseButton,SceneGraphConnection::Connection)),this,SLOT(ConnPressed(Qt::MouseButton,SceneGraphConnection::Connection)));
    connect(m_pOutConn,SIGNAL(connClicked(Qt::MouseButton,SceneGraphConnection::Connection)),this,SLOT(ConnPressed(Qt::MouseButton,SceneGraphConnection::Connection)));

    feather::qml::command::get_node_icon(m_node,m_imgFile);
    m_imgPath << m_imgDir << m_imgFile;
}

SceneGraphNode::~SceneGraphNode()
{

}

// update the node's visual selection
void SceneGraphNode::drawSelected(bool selected)
{
    if(true)
        m_nodeFillBrush.setColor(QColor(SELECTED_NODE_COLOR));
    else 
        m_nodeFillBrush.setColor(QColor(DESELECTED_NODE_COLOR));
    update();
}

void SceneGraphNode::ConnPressed(Qt::MouseButton button, SceneGraphConnection::Connection conn)
{
    ConnClicked(button,conn,m_uid,m_node); 
}

// check SG to see if the node is selected
void SceneGraphNode::setNodeSelection(int type, int uid, int nid)
{
    (feather::smg::Instance()->selected(m_uid)) ? drawSelected(true) : drawSelected(false);
}

void SceneGraphNode::paint(QPainter* painter)
{
    painter->setRenderHints(QPainter::Antialiasing, true);

    QPen trimPen = QPen(QColor(0,0,0),2);
    QPen textPen = QPen(QColor(NODE_TEXT_COLOR),2);
    QFont textFont("DejaVuSans",12);
    textFont.setBold((feather::smg::Instance()->selected(m_uid)) ? true : false);
    //textFont.setBold(true);

    QFont connFont("DejaVuSans",8);

    QBrush connInFillBrush = QBrush(QColor("#FF4500"));
    QBrush connOutFillBrush = QBrush(QColor("#DA70D6"));

    // draw the node box
    painter->setPen(trimPen);
    painter->setBrush(m_nodeFillBrush);

    int incount = feather::qml::command::get_in_field_count(m_uid);
    int outcount = feather::qml::command::get_out_field_count(m_uid);
    int fcount = feather::qml::command::get_field_count(m_uid);
    int height = 20 + (20 * std::max(incount,outcount));
    
    setHeight(height+4);

    painter->drawRoundedRect(QRect(2,2,NODE_WIDTH,height),2,2);

    // draw the input and output connectors
    //QPoint sConnPoint;
    //QPoint tConnPoint;
    //QPoint point; 
    //getConnectionPoint(feather::field::connection::In,point,sConnPoint);
    //getConnectionPoint(feather::field::connection::Out,point,tConnPoint);
    //painter->setBrush(connInFillBrush);
    //painter->drawEllipse(sConnPoint,6,6);
    //painter->setBrush(connOutFillBrush);
    //painter->drawEllipse(tConnPoint,6,6);


    // Node Label 
    painter->setPen(textPen);
    painter->setFont(textFont);
    painter->drawText(QRect(0,0,NODE_WIDTH,height),Qt::AlignHCenter|Qt::AlignTop,feather::qml::command::get_node_name(m_uid).c_str());

    // FOR NOW WE ARE ONLY GOING TO SO THE NODE
    // BUT IN THE FUTURE WE'LL SHOW ALL IT'S 
    // FIELDS
    // in connections
    painter->setFont(connFont);
    int inx=0;
    int outx=0;
    for(int i=1; i <= fcount; i++){
        QString name;
        name = FieldModel::getFieldName(m_node,i);
        if(feather::qml::command::get_field_connection_type(m_uid,i)==feather::field::connection::In){
            painter->drawText(QRect(4,inx+20,NODE_WIDTH-4,NODE_HEIGHT),Qt::AlignLeft|Qt::AlignVCenter,name.toStdString().c_str());
            inx+=20;
        } else {
            painter->drawText(QRect(4,outx+20,NODE_WIDTH-4,NODE_HEIGHT),Qt::AlignRight|Qt::AlignVCenter,name.toStdString().c_str());
            outx+=20;
        }
    }

    // out connections 
    //painter->drawText(QRect(4,20,NODE_WIDTH-4,NODE_HEIGHT),Qt::AlignRight|Qt::AlignVCenter,"output");

    //setX(m_x);
    //setY(m_y);

    // Node Icon
    QRectF tgt(NODE_WIDTH/2-12,14,24,24);
    //QRectF tgt(NODE_WIDTH-26,4,24,24);
    QRectF src(0,0,48,48);
    QImage img(m_imgPath.str().c_str());
    painter->drawImage(tgt,img,src);
}

void SceneGraphNode::mousePressEvent(QMouseEvent* event)
{
    m_x = event->screenPos().x();
    m_y = event->screenPos().y();
    // had nodePressed() here but found out that it would
    // block mouseMoveEvent from ever being called
}

void SceneGraphNode::mouseDoubleClickEvent(QMouseEvent* event)
{
    emit nodePressed(event->button(),m_uid,m_node);
}

void SceneGraphNode::mouseReleaseEvent(QMouseEvent* event)
{
    // this is triggered but not currently used
}

void SceneGraphNode::hoverEnterEvent(QHoverEvent* event)
{
    m_nodeFillBrush.setColor(QColor(HOVER_NODE_COLOR));
    update();
}

void SceneGraphNode::hoverLeaveEvent(QHoverEvent* event)
{
    if(feather::smg::Instance()->selected(m_uid)){
        m_nodeFillBrush.setColor(QColor(SELECTED_NODE_COLOR));
    } else {
        m_nodeFillBrush.setColor(QColor(DESELECTED_NODE_COLOR));
    }
   update();
}

void SceneGraphNode::mouseMoveEvent(QMouseEvent* event)
{
    setX(x() + (event->screenPos().x() - m_x));
    setY(y() + (event->screenPos().y() - m_y));
    m_x = event->screenPos().x();
    m_y = event->screenPos().y();
    parentItem()->update();
}

void SceneGraphNode::inConnectionPoint(QPointF& point)
{
    point = mapToItem(parentItem(),QPoint(m_pInConn->x(),m_pInConn->y()));
}

void SceneGraphNode::outConnectionPoint(QPointF& point)
{
    point = mapToItem(parentItem(),QPoint(m_pOutConn->x(),m_pOutConn->y()));
}


void SceneGraphNode::getConnectionPoint(feather::field::connection::Type conn, QPoint& npoint, QPoint& cpoint)
{
    if(conn == feather::field::connection::In)
    {
        cpoint.setX(npoint.x()+5);
        cpoint.setY((npoint.y()+5)+((NODE_HEIGHT+5)/2));
 
    }
    else
    {
        cpoint.setX((npoint.x()+5)+(NODE_WIDTH+5));
        cpoint.setY((npoint.y()+5)+((NODE_HEIGHT+5)/2));
    }
}


// Link

SceneGraphLink::SceneGraphLink(SceneGraphNode* snode, SceneGraphNode* tnode, QQuickItem* parent) : QQuickPaintedItem(parent), m_snode(snode), m_tnode(tnode)
{

}

SceneGraphLink::~SceneGraphLink()
{

}

void SceneGraphLink::paint(QPainter* painter)
{
    QPainterPath path;
    QBrush brush = painter->brush();
    brush.setStyle(Qt::NoBrush);

    QPointF sp;
    QPointF tp;
    m_snode->outConnectionPoint(sp);
    m_tnode->inConnectionPoint(tp);

    QPen pathPen;
    if(SGState::mode==SGState::Normal)
        pathPen = QPen(QColor(DESELECTED_CONNECTION_COLOR),1);
    else
        pathPen = QPen(QColor(SELECTED_CONNECTION_COLOR),2);

    path.moveTo(sp.x()+2,sp.y()+2);

    int midX = sp.x() + ((tp.x()-sp.x())/2);

    if(SGState::mode==SGState::Normal)
        path.cubicTo(midX,sp.y(),
                midX,tp.y(),
                tp.x()-2,tp.y()+2);
    else 
        path.cubicTo(MouseInfo::clickX,sp.y(),
                midX,MouseInfo::clickY-35,
                MouseInfo::clickX-2,MouseInfo::clickY-35);

    painter->setPen(pathPen);
    painter->drawPath(path);
}


// Editor

SceneGraphEditor::SceneGraphEditor(QQuickItem* parent) : QQuickPaintedItem(parent), m_scale(100), m_nodeWidth(80), m_nodeHeight(30)
{
    SGState::pSge = this;
    setAcceptedMouseButtons(Qt::AllButtons);

    // for testing purposes I'm selecting the node from here.
    // later this will be done from the viewport or outliner
    feather::qml::command::select_node(0,0);
    updateGraph();
}

SceneGraphEditor::~SceneGraphEditor()
{

}

void SceneGraphEditor::ConnOption(Qt::MouseButton button, SceneGraphConnection::Connection conn, int uid, int nid)
{
    feather::field::FieldBase* pfield;
    int i=1;
    feather::qml::command::get_field_base(uid,i,pfield);

    m_connection->clear();
 
    while(pfield!=NULL)
    {
        if(conn == SceneGraphConnection::In) {
            if(pfield->conn_type == feather::field::connection::In)
                m_connection->addField(uid,nid,i,pfield->type,true); 
        }

        if(conn == SceneGraphConnection::Out) {
            if(pfield->conn_type == feather::field::connection::Out)
                m_connection->addField(uid,nid,i,pfield->type,true); 
        }

        i++;
        feather::qml::command::get_field_base(uid,i,pfield);
    }

    m_connection->layoutChanged();
    openConnMenu();
}

void SceneGraphEditor::nodePressed(Qt::MouseButton button, int uid, int nid)
{
    // for now we'll have it so only one node can be selected at a time
    feather::qml::command::clear_selection();
    //feather::qml::command::select_node(uid);
    emit nodeSelection(0,uid,nid);
}

void SceneGraphEditor::connectionMousePressed(int button, int uid, int nid, int fid)
{
    std::cout << "connection mouse pressed, button " << button << " uid " << uid << " nid " << nid << " fid " << fid << std::endl;
}

void SceneGraphEditor::connectionMouseReleased(int button, int uid, int nid, int fid)
{
    std::cout << "connection mouse released, button " << button << " uid " << uid << " nid " << nid << " fid " << fid << std::endl;
}

void SceneGraphEditor::connectionMouseClicked(int button, int uid, int nid, int fid)
{
    std::cout << "connection mouse clicked, button " << button << " uid " << uid << " nid " << nid << " fid " << fid << std::endl;
    
    if(SGState::mode==SGState::Normal)
        SGState::mode=SGState::FieldConnection;
    else
        SGState::mode=SGState::Normal;
}

void SceneGraphEditor::paint(QPainter* painter)
{
    setFillColor(QColor(BACKGROUND_COLOR));
    painter->setRenderHints(QPainter::Antialiasing, true);

    std::for_each(m_links.begin(),m_links.end(),[painter](SceneGraphLink* l){ l->paint(painter); });
}

void SceneGraphEditor::updateGraph()
{
    int xpos = 50;
    int ypos = 50;

    std::for_each(m_nodes.begin(), m_nodes.end(), [](SceneGraphNode* node){ delete node; });
    std::for_each(m_links.begin(), m_links.end(), [](SceneGraphLink* link){ delete link; });

    m_nodes.clear();
    m_links.clear();

    std::vector<int> uids;
    // disabled selection as root for testing
    //feather::qml::command::get_selected_nodes(uids);
    uids.push_back(0);

    std::cout << uids.size() << " nodes are selected\n";

    std::for_each(uids.begin(),uids.end(),[](int& n){ std::cout << n << ","; });

    // for each selected uid we will draw all the nodes connected to it.
    for(uint i=0; i < uids.size(); i++) {
        updateLeaf(NULL,uids[i],xpos,ypos);
    }
}


void SceneGraphEditor::updateLeaf(SceneGraphNode* pnode, int uid, int xpos, int ypos)
{
    int nid=0;
    feather::status s = feather::qml::command::get_node_id(uid,nid);

    SceneGraphNode *node = new SceneGraphNode(uid,nid,this);

    node->setX(xpos);
    node->setY(ypos);

    connect(node,SIGNAL(ConnClicked(Qt::MouseButton,SceneGraphConnection::Connection,int,int)),this,SLOT(ConnOption(Qt::MouseButton,SceneGraphConnection::Connection,int,int)));
    connect(node,SIGNAL(nodePressed(Qt::MouseButton,int,int)),this,SLOT(nodePressed(Qt::MouseButton,int,int)));
    connect(this,SIGNAL(nodeSelection(int,int,int)),node,SLOT(setNodeSelection(int,int,int)));

    m_nodes.push_back(node);
    
    // make a link if we have a pointer to a parent node
    if(pnode!=NULL) {
        std::cout << "adding link\n";
        m_links.push_back(new SceneGraphLink(pnode,node,this));
    }

    std::vector<int> cuids;
    // update each connected node as a separate leaf 
    feather::qml::command::get_node_connected_uids(uid,cuids);
    int ystep = 0;
    std::for_each(cuids.begin(),cuids.end(),[&xpos,&ypos,this,node,&ystep](int key){
        std::cout << "drawing leaf " << key << std::endl;
        updateLeaf(node,key,xpos+200,ypos+ystep);
        ystep+=40;
    });
}


void SceneGraphEditor::mousePressEvent(QMouseEvent* event){ std::cout << "mouse press\n"; };
void SceneGraphEditor::mouseReleaseEvent(QMouseEvent* event){ std::cout << "mouse release\n"; };
void SceneGraphEditor::hoverEnterEvent(QHoverEvent* event){ std::cout << "hover enter\n"; };
void SceneGraphEditor::hoverLeaveEvent(QHoverEvent* event){ std::cout << "hover leave\n"; };
void SceneGraphEditor::hoverMoveEvent(QHoverEvent* event){ std::cout << "hover move\n"; };
void SceneGraphEditor::mouseMoveEvent(QMouseEvent* event){ std::cout << "mouse move\n"; };

