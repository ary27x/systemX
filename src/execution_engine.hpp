
#ifndef __EXECUTION_ENGINE_H
#define __EXECUTION_ENGINE_H

#include <iostream>
#include <string>
#include "frontend.hpp"
#include "pager.hpp"
// #include "lru_cache.hpp"


struct server_details
{
    std::string ipv4_address;
    std::string port_number;
    server_details(std::string _ipv4_address , std::string _port_number)
    {
        this->ipv4_address = _ipv4_address;
        this->port_number = _port_number;
    } 
};

class execution_engine
{
    private :
    bool connected_to_server;
    server_details * server_info;
    std::string current_db;

    // this is basically the set of all the command which can be run without "employing" a database
    std::vector<NODE_SET> independant_commands = {NODE_CREATE_DATABASE , NODE_USE_DATABASE , NODE_SERVER_CONNECT , NODE_SERVER_CREATE , NODE_EXIT , NODE_EXPORT};
    
    public :
    Pager * main_pager;
    int command_counter;

    execution_engine()
    {
        main_pager = new Pager();
        connected_to_server = false;
        server_info = nullptr;
        current_db = "";
        command_counter = 0;
    }

    void generate_records(uint64_t start , uint64_t data , std::string _table)
    {
    std::time_t start_time = std::time(nullptr);
    std::cout << "Start time : " <<  start_time << std::endl;

    std::string * table_name = new std::string(_table);
    AST_NODE * action_node = new AST_NODE();
    action_node->PAYLOAD = table_name;


    // Batch size set to 1 million
    
    
    std::vector<std::vector<AST_NODE *>> multi_data_records;

        for (uint64_t itr = start; itr <= data; itr++)
        {
            std::vector<AST_NODE *> buffer_vector;
            // int id, string name, int age
            AST_NODE * id_node = new AST_NODE();
            AST_NODE * name_node = new AST_NODE();
            AST_NODE * age_node = new AST_NODE();

            id_node->NODE_TYPE = NODE_INT;
            name_node->NODE_TYPE = NODE_STRING;
            age_node->NODE_TYPE = NODE_INT;

            std::string * id_value = new std::string(std::to_string(itr));
            std::string * name_value = new std::string("n" + std::to_string(itr));
            std::string * age_value = new std::string(std::to_string(itr));

            id_node->PAYLOAD = id_value;
            name_node->PAYLOAD = name_value;
            age_node->PAYLOAD = age_value;

            buffer_vector.push_back(id_node);
            buffer_vector.push_back(name_node);
            buffer_vector.push_back(age_node);

            multi_data_records.push_back(buffer_vector);
        }

        action_node->MULTI_DATA = multi_data_records;
        main_pager->add_to_heap(action_node);

        // After every batch, clear multi_data_records to avoid memory issues

         for (auto &record : multi_data_records)
        {
            for (AST_NODE *node : record)
            {
                delete static_cast<std::string *>(node->PAYLOAD); // Delete the payload
                delete node;                                      // Delete AST_NODE
            }
        }
        multi_data_records.clear();

        

    std::time_t end_time = std::time(nullptr);
    std::cout << "End time : " <<  end_time << std::endl;

    std::cout << "Time taken : " << end_time - start_time << " seconds "<< std::endl;
}

    bool handle_new(AST_NODE *& action_node)
    {
        std::time_t start_time = std::time(nullptr);
        std::string table_name_copy = *action_node->PAYLOAD;
        std::string append_db_name = this->current_db + "." + *action_node->PAYLOAD;
        action_node->PAYLOAD = &append_db_name;
        
        bool status = main_pager->create_new_heap(action_node);
        if (!status)
        {
            std::cout << FAIL << "\n[!] ERROR : Table Already Exists : " << *action_node->PAYLOAD<< std::endl;        
            std::cout << FAIL << "$ Command ID -> " << this->command_counter << " failed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
            return false;
        }
            
        std::ofstream write_stream("bluedb_" + this->current_db + "_metadata.txt" , std::ios::app | std::ios::out);
        write_stream << table_name_copy << "\n";
        write_stream.close();

        std::cout << SUCCESS << "\n[*] Table Created : " << *action_node->PAYLOAD << std::endl;        
        std::cout << SUCCESS << "$ Command ID -> " << this->command_counter << " executed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
        return true;
    }

    bool handle_add(AST_NODE *& action_node)
    {
        std::time_t start_time = std::time(nullptr);
        std::string name_copy = *action_node->PAYLOAD;
        std::string append_db_name = this->current_db + "." + *action_node->PAYLOAD;
        action_node->PAYLOAD = &append_db_name; 

        bool status = main_pager->add_to_heap(action_node);
        if (!status)
        {
            std::cout << FAIL << "$ Command ID -> " << this->command_counter << " failed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
            return false;
        }

        std::cout << SUCCESS << "\n[*] Data Inserted Successfully"  << DEFAULT << std::endl;

        AST_NODE * print_node = new AST_NODE();
        action_node->PAYLOAD = &name_copy; // this is to prevent double db name insertion
        print_node->DATA_LIST.push_back(*action_node->PAYLOAD);

        handle_print(print_node , start_time);
        delete print_node;
        return true;

    }
    
    bool handle_print(AST_NODE *& action_node , std::time_t parent_function_start_time = 0)
    {
        std::time_t start_time;
        if (parent_function_start_time == 0)
            start_time = std::time(nullptr);
        else
            start_time = parent_function_start_time;

        action_node->DATA_LIST[0] = this->current_db + "." + action_node->DATA_LIST[0];

        bool status = main_pager->get_heap(action_node);
        if (!status)
        {
            std::cout << FAIL << "$ Command ID -> " << this->command_counter << " failed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
            return false;
        }

        std::cout << SUCCESS << "\n$ Command ID -> " << this->command_counter << " executed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
        return true;

    }
    
    bool handle_update(AST_NODE *& action_node)
    {
        std::time_t start_time = std::time(nullptr);
        std::string name_copy = *action_node->PAYLOAD;

        std::string append_db_name = this->current_db + "." + *action_node->PAYLOAD;
        action_node->PAYLOAD = &append_db_name;

        bool status = main_pager->update_heap(action_node);
        if (!status)
        {
            std::cout << FAIL << "$ Command ID -> " << this->command_counter << " failed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
            return false;
        }

        std::cout << SUCCESS << "\n[*] Table Updated Successfully "  << DEFAULT << std::endl;

        AST_NODE * print_node = new AST_NODE();
        action_node->PAYLOAD = &name_copy; // this is to prevent double db name insertion
        print_node->DATA_LIST.push_back(*action_node->PAYLOAD);

        handle_print(print_node , start_time);
        delete print_node;

        return true;


    }
    
    bool handle_remove(AST_NODE *& action_node)
    {
        std::time_t start_time = std::time(nullptr);
        std::string name_copy = *action_node->PAYLOAD;

        std::string append_db_name = this->current_db + "." + *action_node->PAYLOAD;
        action_node->PAYLOAD = &append_db_name;

        bool status = main_pager->delete_from_heap(action_node);
        if (!status)
        {
            std::cout << FAIL << "$ Command ID -> " << this->command_counter << " failed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
            return false;
        }

        std::cout << SUCCESS << "\n[*] Deletion Performed Successfully "  << DEFAULT << std::endl;

        AST_NODE * print_node = new AST_NODE();
        action_node->PAYLOAD = &name_copy; // this is to prevent double db name insertion

        print_node->DATA_LIST.push_back(*action_node->PAYLOAD);

        handle_print(print_node , start_time);
        delete print_node;
        return true;
    }

    bool handle_exit(AST_NODE *& action_node)
    {
        std::cout << FAIL << "\n[~] Exiting SystemX" << DEFAULT <<  std::endl;
        exit(0);
    }

    bool handle_create_server( AST_NODE *& action_node)
    {
        std::cout << "creating the server on the address : " << *action_node->PAYLOAD << std::endl;
        std::cout << "port number : " << *action_node->SUB_PAYLOAD << std::endl;
        return true;
        
    }
    
    bool handle_connect_server(AST_NODE *& action_node)
    {
        std::cout << "Connect to the server : " << std::endl;
        std::cout << "Address of the server : " << *action_node->PAYLOAD << std::endl;
        std::cout << "Port number of the server : " << *action_node->SUB_PAYLOAD << std::endl;
        this->connected_to_server = true;
        this->server_info = new server_details(*action_node->PAYLOAD , *action_node->SUB_PAYLOAD);
        return true;
    }

    bool push_to_server(AST_NODE *& action)
    {
        std::cout << "the following command would be run on the server and the response would be printed here : " << std::endl;
        return true;
    }

    bool handle_create_database(AST_NODE *& action_node)
    {
        std::time_t start_time = std::time(nullptr);
        std::ofstream write_stream("bluedb_database_list.txt" , std::ios::app | std::ios::out);
        write_stream << *action_node->PAYLOAD << "\n";
        write_stream.close();

        std::cout << SUCCESS << "\n[*] Database Created : " << *action_node->PAYLOAD << std::endl;
        std::cout << SUCCESS << "$ Command ID -> " << this->command_counter << " executed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
        return true;

    }
    
    bool handle_use_database(AST_NODE *& action_node)
    {
        std::time_t start_time = std::time(nullptr);
        std::ifstream read_stream("bluedb_database_list.txt");
        std::string existing_db_name;
        while (std::getline(read_stream , existing_db_name))
        {
            if (*action_node->PAYLOAD == existing_db_name)
            {
                this->current_db = *action_node->PAYLOAD;
                read_stream.close();

                std::cout << SUCCESS << "\n[*] Working On Database : " << *action_node->PAYLOAD << std::endl;
                std::cout << SUCCESS << "$ Command ID -> " << this->command_counter << " executed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
                
                return true;
            }
        }
        read_stream.close();
        std::cout << FAIL << "\n[!] ERROR : The Database Was Not Found : " << *action_node->PAYLOAD << std::endl;
        std::cout << FAIL << "$ Command ID -> " << this->command_counter << " failed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
        return false;
    }

    bool handle_export_database(AST_NODE *& action_node)
    {
        std::time_t start_time = std::time(nullptr);
        std::ifstream read_stream("bluedb_database_list.txt");
        std::string existing_db_name;
        bool found = false;
        while (std::getline(read_stream , existing_db_name))
        {
            if (*action_node->PAYLOAD == existing_db_name)
            {
                this->current_db = *action_node->PAYLOAD;
                read_stream.close();
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::cout << FAIL << "\n[!] ERROR : Choose Not Find Database : " << *action_node->PAYLOAD << std::endl;
            std::cout << FAIL << "$ Command ID -> " << this->command_counter << " failed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
            return false;
        }
        
        std::ifstream table_read_stream("bluedb_" + *action_node->PAYLOAD + "_metadata.txt");
        std::string table_name;

        std::ofstream mysql_write_stream(this->current_db + ".sql");

        mysql_write_stream << "CREATE DATABASE " << this->current_db  << ";" << std::endl << std::endl;
        mysql_write_stream << "USE " << this->current_db  << ";" << std::endl << std::endl;

        while (std::getline(table_read_stream , table_name))
        {
            mysql_write_stream << "CREATE TABLE " << table_name << " (\n";

            std::ifstream heap_read_stream (this->current_db + "." + table_name + ".dat" , std::ios::binary);
            
            HeapFile_Metadata current_heapfile_metadata = main_pager->deserialize_heapfile_metadata<std::ifstream>(heap_read_stream);
            std::vector<std::string> schema_chunks = main_pager->split_schema(current_heapfile_metadata.schema);

            
            for (int itr = 0 ; itr < schema_chunks.size() ; itr++)
            {
                std::string current_attribute = schema_chunks[itr];
                switch(current_attribute[0])
                {
                    case 's' : 
                    {
                        mysql_write_stream << main_pager->get_string_name_from_chunk(current_attribute) << " VARCHAR(";
                        mysql_write_stream << main_pager->get_string_size_from_chunk(current_attribute) << ")";
                        break;
                    }
                    case 'i' :
                    {
                        mysql_write_stream << current_attribute.substr(1) << " INT";
                        break;
                    }
                    case 'f' : 
                    {
                        mysql_write_stream << current_attribute.substr(1) << " FLOAT";
                        break;
                    }
                }
                if (current_heapfile_metadata.primary_key_string[itr] == '1')
                    mysql_write_stream << " PRIMARY KEY";

                if (itr != schema_chunks.size() - 1)
                    mysql_write_stream << ",";
                
                mysql_write_stream << "\n";
            }
            heap_read_stream.close();
            
            mysql_write_stream << ");\n\n";

            if (current_heapfile_metadata.record_count != 0)
            {
                mysql_write_stream << "INSERT INTO " << table_name << " VALUES\n";
                main_pager->export_deserialization(this->current_db + "." + table_name , mysql_write_stream);
            }
            
        }

        mysql_write_stream.close();
        table_read_stream.close();

        std::cout << SUCCESS << "\n[*] Database Exported As : " << *action_node->PAYLOAD << ".sql" << std::endl;
        std::cout << SUCCESS << "$ Command ID -> " << this->command_counter << " executed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
        return true;

    }

    bool check_for_db_usage()
    {
        if (this->current_db == "")
            return false;
        return true;
    }
    
    bool execute (AST_NODE *& action_node)
    {
        this->command_counter++;
        std::time_t start_time = std::time(nullptr);

        if (connected_to_server)
        {
            push_to_server(action_node);
            return true;
        }
        
        bool is_independant_command = false;
        for (NODE_SET itr : independant_commands)
            if (itr == action_node->NODE_TYPE)
                is_independant_command = true;
        
        if (!is_independant_command)
        {
            if (!check_for_db_usage()) // db is not selected for a dependant command
            {
                std::cout << FAIL << "\n[!] ERROR : Choose A Database To Perform The Operation On" << std::endl;
                std::cout << FAIL << "$ Command ID -> " << this->command_counter << " failed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
                return false;
            }
        }

  
        switch(action_node->NODE_TYPE)
        {
            case NODE_NEW             : return this->handle_new(action_node);
            case NODE_ADD             : return this->handle_add(action_node);
            case NODE_PRINT           : return this->handle_print(action_node);
            case NODE_UPDATE          : return this->handle_update(action_node);
            case NODE_REMOVE          : return this->handle_remove(action_node);
            case NODE_EXIT            : return this->handle_exit(action_node);
            case NODE_SERVER_CREATE   : return this->handle_create_server(action_node);
            case NODE_SERVER_CONNECT  : return this->handle_connect_server(action_node);
            case NODE_CREATE_DATABASE : return this->handle_create_database(action_node);
            case NODE_USE_DATABASE    : return this->handle_use_database(action_node); 
            case NODE_EXPORT          : return this->handle_export_database(action_node); 
        }
        return true;

    }
};


/*

std::cout << FAIL << "\n[!] ERROR : Choose A Database To Perform The Operation On" << std::endl;
std::cout << FAIL << "$ Command ID -> " << this->command_counter << " failed in " << std::time(nullptr) - start_time << " sec\n\n" << DEFAULT;
return false;

*/
#endif
