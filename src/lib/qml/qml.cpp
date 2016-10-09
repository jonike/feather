/***********************************************************************
 *
 * Filename: qml.cpp
 *
 * Description: Frontend of the QML wrapper.
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

#include "qml.hpp"
#include "parameter.hpp"
#include "field.hpp"
#include "plugin.hpp"

using namespace feather;

Connection::Connection(QObject* parent)
{
}

Connection::~Connection()
{
}

// SceneGraph
SceneGraph::SceneGraph(QObject* parent)
{
}

SceneGraph::~SceneGraph()
{
}

void SceneGraph::clear()
{
    qml::command::clear();
    emit cleared();
}

int SceneGraph::add_node(int node, QString name)
{
    int uid = qml::command::add_node(node,name.toStdString());
    emit nodeAdded(uid);
    emit nodeAddDrawItems(uid);
    return uid;
}

bool SceneGraph::remove_node(int uid)
{
    status e;
    qml::command::remove_node(uid,e);
    emit nodeRemoved(uid);
    return e.state;
}

QString SceneGraph::node_name(int uid)
{
    std::string name;
    status e;
    qml::command::get_node_name(uid,name,e);
    return QString(name.c_str());
}

int SceneGraph::get_node_by_name(QString name)
{
    unsigned int uid=0;
    qml::command::get_node_by_name(name.toStdString(),uid);
    return uid;
}

int SceneGraph::node_id(int uid)
{
    status e;
    return qml::command::get_node_id(uid,e);
}

int SceneGraph::connect_nodes(int n1, int f1, int n2, int f2)
{
    status p = qml::command::connect_nodes(n1,f1,n2,f2);
    //emit nodeUpdateDrawItems(n2);
    if(p.state==FAILED)
        std::cout << p.msg << std::endl;
    return p.state;
}

int SceneGraph::selected_node()
{
    return qml::command::selected_node();
}
   
int SceneGraph::select_node(int uid)
{
    status p = qml::command::select_node(uid);
    emit nodeSelected();
    return p.state;
}
 
int SceneGraph::select_node(int type, int uid)
{
    status p = qml::command::select_node(type,uid);
    emit nodeSelected();
    return p.state;
}

int SceneGraph::select_field(int type, int uid, int fid)
{
    status p = qml::command::select_node(type,uid,fid);
    emit fieldSelected(uid,fid);
    return p.state;
}

void SceneGraph::clear_selection()
{
    qml::command::clear_selection();
}

int SceneGraph::run_command_string(QString str)
{
    status p = qml::command::run_command_string(str.toStdString());
    emit commandMessageChanged(p.state,QString(p.msg.c_str()));
    std::cout << "run command string msg='" << p.msg << "'\n";
    return p.state;
}

void SceneGraph::triggerUpdate()
{
    qml::command::scenegraph_update();
    // some widgets need to know if nodes where added and what was added

    std::vector<unsigned int> uids;
    bool n = feather::qml::command::nodes_added(uids);

    std::cout << "triggerUpdate saw that the following uids where added\n";

    if(n){ 
        for(auto uid : uids)
            emit nodeAdded(uid);
    }

    emit updateGraph();
}

void SceneGraph::add_node_to_layer(int uid, int lid)
{
    qml::command::add_node_to_layer(uid,lid);
}

bool SceneGraph::connected(unsigned int uid, unsigned int fid)
{
    status e;
    bool conn;
    unsigned int nid = qml::command::get_node_id(uid,e);
    qml::command::get_field_connection_status(uid,nid,fid,conn);
    return conn; 
}

QList<int> SceneGraph::connected_uids(unsigned int uid, unsigned int fid)
{
    QList<int> list;
    std::vector<int> uids;
    feather::plugin::get_node_connected_uids(uid,fid,uids);
    std::cout << "connected_uids(" << uid << "," << fid << ") uids count = " << uids.size() << std::endl;
    for(auto uid : uids) {
        std::cout << "added uid " << uid << " to list\n";
        list.push_back(uid);
    }
    return list;
}

// Field
Field::Field(QObject* parent): m_uid(0),m_nid(0),m_fid(0),m_boolVal(false),m_intVal(0),m_realVal(0.0),m_connected(false)
{
}

Field::~Field()
{
}

int Field::type()
{
    int type=0;
    qml::command::get_field_type(m_uid,m_nid,m_fid,type);
    return type;
}

// GET FIELD VALUES

void Field::get_bool_val()
{
    qml::command::get_field_val(m_uid,m_nid,m_fid,m_boolVal);
}

void Field::get_int_val()
{
    qml::command::get_field_val(m_uid,m_nid,m_fid,m_intVal);
}

void Field::get_real_val()
{
    qml::command::get_field_val(m_uid,m_nid,m_fid,m_realVal);
}

// SET FEILD VALUES

void Field::set_bool_val()
{
    qml::command::set_field_val(m_uid,m_nid,m_fid,m_boolVal);
}

void Field::set_int_val()
{
    qml::command::set_field_val(m_uid,m_nid,m_fid,m_intVal);
}

void Field::set_real_val()
{
    qml::command::set_field_val(m_uid,m_nid,m_fid,m_realVal);
}

void Field::set_real_array_val()
{
    FRealArray value;

    int i=0;
    for ( auto val : m_realArrayVal){
        std::cout << "setting weight " << i << " to " << val << std::endl;
        value.push_back(val);
        i++;
    }
    qml::command::set_field_val(m_uid,m_nid,m_fid,value);
}

// Real Array

//QQmlListProperty<double> Field::realArrayVal()
QList<double> Field::realArrayVal()
{
    m_realArrayVal.clear();

    // TODO - put a check here to verify that this is actually a realArray field

    field::Field<FRealArray>* array = static_cast<field::Field<FRealArray>*>(plugin::get_node_field_base(m_uid,m_fid));

    for(auto val : array->value){
        m_realArrayVal.append(val);
    }
 
    return m_realArrayVal;     
    //return QQmlListProperty<double>(this,m_realArrayVal);     

    /*
    std::vector<field::Connection> connections;
    plugin::connections(m_uid,m_fid,connections);
 
    for ( auto connection : connections ) {
        Connection* conn = new Connection();
        conn->setSuid(connection.puid);
        conn->setSfid(connection.pfid);
        conn->setTuid(m_uid);
        conn->setTfid(m_fid);
        m_connections.append(conn);
    }

    return QQmlListProperty<Connection>(this,m_connections);
    */
}


// GET CONNECTED

void Field::get_connected()
{
    qml::command::get_field_connection_status(m_uid,m_nid,m_fid,m_connected);
}

QQmlListProperty<Connection> Field::connections()
{
    m_connections.clear();

    std::vector<field::Connection> connections;
    plugin::connections(m_uid,m_fid,connections);
 
    for ( auto connection : connections ) {
        Connection* conn = new Connection();
        conn->setSuid(connection.puid);
        conn->setSfid(connection.pfid);
        conn->setTuid(m_uid);
        conn->setTfid(m_fid);
        m_connections.append(conn);
    }

    return QQmlListProperty<Connection>(this,m_connections);
}

void Field::setFid(unsigned int& fid) {
    if ( m_fid != fid ) {
        m_fid = fid;
    }
}


// Node
Node::Node(QObject* parent):
    m_uid(0),
    m_nid(0)
{
}

Node::~Node()
{
}

/*
QQmlListProperty<Field> Node::inFields()
{
    return QQmlListProperty<Field>(this,0,&Node::append_inField,0,0,0);
}

QQmlListProperty<Field> Node::outFields()
{
    return QQmlListProperty<Field>(this,0,&Node::append_outField,0,0,0);
}

void Node::append_inField(QQmlListProperty<Field> *list, Field *field)
{
    Node *node = qobject_cast<Node*>(list->object);
    if(node) {
        node->m_inFields.append(field);
    }
}

void Node::append_outField(QQmlListProperty<Field> *list, Field *field)
{
    Node *node = qobject_cast<Node*>(list->object);
    if(node) {
        node->m_outFields.append(field);
    }
}
*/

unsigned int Node::field_count()
{
    return qml::command::get_field_count(m_uid); 
}

unsigned int Node::in_field_count()
{
    return qml::command::get_in_field_count(m_uid); 
}

unsigned int Node::out_field_count()
{
    return qml::command::get_out_field_count(m_uid); 
}

QList<unsigned int> Node::in_fields()
{
    std::vector<field::FieldBase*> fids;
    QList<unsigned int> qfids;

    qml::command::get_fid_list(m_uid,m_nid,field::connection::In,fids);
    for(auto f : fids)
        qfids.push_back(f->id);
 
    return qfids;
}

QList<unsigned int> Node::out_fields()
{
    std::vector<field::FieldBase*> fids;
    QList<unsigned int> qfids;

    qml::command::get_fid_list(m_uid,m_nid,field::connection::Out,fids);
    for(auto f : fids)
        qfids.push_back(f->id);
    
    return qfids;
}

// Parameter
Parameter::Parameter(QObject* parent) : QObject(parent)
{
}

Parameter::~Parameter()
{
}


// Command
Command::Command(QObject* parent) : QObject(parent)
{
}

Command::~Command()
{
}

bool Command::exec()
{
    parameter::ParameterList params;
    for(int i=0; i < m_parameters.size(); i++)
    {
        Parameter* p = m_parameters.at(i);

        switch(p->type())
        {
            case Parameter::Bool:
                params.addBoolParameter(p->name().toStdString(), p->boolValue()); 
                break;

            case Parameter::Int:
                params.addIntParameter(p->name().toStdString(), p->intValue()); 
                break;

            case Parameter::Real:
                params.addRealParameter(p->name().toStdString(), p->realValue()); 
                break;

            case Parameter::String:
                params.addStringParameter(p->name().toStdString(), p->stringValue().toStdString()); 
                break;

            default:
                break; 
        }

    }

    qml::command::run_command(m_name.toStdString(), params);  

    return false;
}


QQmlListProperty<Parameter> Command::parameters()
{
    return QQmlListProperty<Parameter>(this,0,&Command::append_parameter,0,0,0);
}

void Command::append_parameter(QQmlListProperty<Parameter> *list, Parameter *parameter)
{
    Command *cmd = qobject_cast<Command*>(list->object);
    if(cmd) {
        cmd->m_parameters.append(parameter);
    }
}


// Plugins
Plugins::Plugins(QObject* parent) : QAbstractListModel(parent)
{
    load();
}

Plugins::~Plugins()
{
}

QHash<int, QByteArray> Plugins::roleNames() const
{

    QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    roles.insert(NameRole, QByteArray("name"));
    roles.insert(DescriptionRole, QByteArray("description"));
    roles.insert(AuthorRole, QByteArray("author"));
    return roles;
}

int Plugins::rowCount(const QModelIndex& parent) const
{
    return m_items.size();
}

QVariant Plugins::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant(); // Return Null variant if index is invalid
    if (index.row() > (m_items.size()-1) )
        return QVariant();
    PluginObject *dobj = m_items.at(index.row());
    switch (role) {
        case Qt::DisplayRole: // The default display role now displays the first name as well
        case NameRole:
            return QVariant::fromValue(dobj->name);
        case DescriptionRole:
            return QVariant::fromValue(dobj->description);
        case AuthorRole:
            return QVariant::fromValue(dobj->author);
        default:
            return QVariant();
    }
}

void Plugins::load()
{
    std::vector<PluginInfo> list;
    qml::command::get_plugins(list);

    for(uint i=0; i < list.size(); i++)
        m_items.append(new PluginObject(QString(list[i].name.c_str()), QString(list[i].description.c_str()), QString(list[i].author.c_str())));
}

