// =====================================================================================
// 
//       Filename:  pluginmanager.cpp
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  11/03/2014 03:23:54 AM
//       Revision:  none
//       Compiler:  g++
// 
//         Author:  Richard Layman (), rlayman2000@yahoo.com
//        Company:  
// 
// =====================================================================================
#include "pluginmanager.hpp"

using namespace feather;

PluginManager::PluginManager():m_pluginPath("~/.feather/plugins")
{

}

PluginManager::~PluginManager()
{
    for(auto n : m_plugins)
        dlclose(n.handle);
}

status PluginManager::load_plugins()
{
    boost::filesystem::path plugin_path("/usr/local/feather/plugins");
    typedef std::vector<boost::filesystem::path> files;
    files plugin_paths;

    if (boost::filesystem::exists(plugin_path))
    {
        if (boost::filesystem::is_regular_file(plugin_path))   
            std::cout << plugin_path << " size is " << boost::filesystem::file_size(plugin_path) << '\n';

        else if (boost::filesystem::is_directory(plugin_path)) {
            std::cout << plugin_path << "is a directory\n";
            std::copy(boost::filesystem::directory_iterator(plugin_path), boost::filesystem::directory_iterator(), std::back_inserter(plugin_paths));
            for (files::const_iterator it (plugin_paths.begin()); it != plugin_paths.end(); ++it)
            {
                if((*it).extension()==".so") {
                    // load plugin here
                    std::cout << "LOADING: " << *it << std::endl;
                    PluginInfo node;
                    node.path = (*it).string();
                    status s = load_node(node);
                    if(s.state){
                        m_plugins.push_back(node);
                        std::cout << node.path << " loaded\n";
                    }
                    else
                        std::cout << node.path << " " << s.msg << std::endl;
                }
            }
        }

        else
            std::cout << plugin_path << "exists, but is neither a regular file nor a directory\n";
    }
    else
        std::cout << plugin_path << "does not exist\n";

    return status();
}

status PluginManager::do_it(int node)
{
    std::cout << "call node " << node << std::endl;
    std::for_each(m_plugins.begin(),m_plugins.end(), call_do_it(node) );
    return status();
}

status PluginManager::load_node(PluginInfo &node)
{
   char *error;

   node.handle = dlopen(node.path.c_str(), RTLD_LAZY);
    if (!node.handle) 
    {
        fprintf(stderr, "%s\n", dlerror());
        return status(FAILED,"loaded failed to load");
    }

    node.get_id = (int(*)())dlsym(node.handle, "get_id");
    node.call_node = (bool(*)(int*))dlsym(node.handle, "call_node");
    node.do_it = (status(*)(int,PluginNodeFields*))dlsym(node.handle, "do_it");
    node.node_match = (bool(*)(int))dlsym(node.handle, "node_match");
    node.add_node = (status(*)(int,PluginNodeFields*))dlsym(node.handle, "add_node");
    node.remove_node = (status(*)(int,PluginNodeFields*))dlsym(node.handle, "remove_node");
    node.get_field = (field::FieldBase*(*)(int,int,PluginNodeFields*))dlsym(node.handle, "get_field");
    node.import_data = (status(*)(int,parameter::ParameterList))dlsym(node.handle, "import_data");
 


    if ((error = dlerror()) != NULL)  
    {
        fprintf(stderr, "%s\n", error);
        return status(FAILED,"library error");
    }

    //std::cout << "node id=" << node.get_id() << std::endl; 
    //(*fn)(&x);
    //printf("Valx=%d\n",x);

    return status();
}

status PluginManager::load_command(PluginInfo &command)
{
    return status(FAILED,"no command to load");
}
