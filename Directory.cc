
#include "jet/Directory.h"
#include "jet/Utf8String.h"
#include "jet/Exception.h"

#include <iostream>
#include <fstream>
#include <string>


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include <unistd.h>



namespace jet{


    Directory::Directory(){

        this->full_path = Directory::getWorkingPath();

    }

    Directory::Directory( Utf8String full_path )
        :full_path(full_path)
    {


    }


    Directory::Directory( const char *full_path )
        :full_path( Utf8String(full_path) )
    {


    }


    bool Directory::exists() const{

        struct stat file_status;
        if( stat( full_path.getCString(), &file_status ) == 0 ){
            return true;
        }
        return false;

    }



    vector<File*>* Directory::getFiles() const{

        vector<File*> *files = new vector<File*>;


        DIR *directory;
        struct dirent *directory_entry;
        File *current_file;
        Utf8String filename;


        directory = opendir( this->full_path.getCString() );
        if( directory != NULL ){

            while( (directory_entry = readdir(directory)) ){

                // Only recognize regular files or symlinks
                if( directory_entry->d_type != (DT_REG|DT_LNK) ){
                    continue;
                }

                filename = this->full_path;
                filename += Utf8String( "/" ) + Utf8String( directory_entry->d_name );

                current_file = new File( filename );
                files->insert( files->end(), current_file );

            }
            closedir(directory);
            return files;

        }else{

            delete files;
            throw new Exception( Utf8String("Couldn't open the directory: ") + this->full_path );

        }

    }


    vector<Directory*>* Directory::getDirectories() const{


        DIR *directory;
        struct dirent *directory_entry;
        Directory *current_directory;
        Utf8String filename;


        directory = opendir( this->full_path.getCString() );
        if( directory != NULL ){

            vector<Directory*> *directories = new vector<Directory*>;

            while( (directory_entry = readdir(directory)) ){

                // Only recognize directories
                if( directory_entry->d_type != DT_DIR ){
                    continue;
                }

                filename = this->full_path;
                filename += Utf8String( "/" ) + Utf8String( directory_entry->d_name );

                current_directory = new Directory( filename );
                directories->insert( directories->end(), current_directory );

            }
            closedir(directory);
            return directories;

        }else{

            throw new Exception( Utf8String("Couldn't open the directory: ") + this->full_path );

        }

    }


    Utf8String Directory::getFullPath() const{

        return this->full_path;

    }


    Utf8String Directory::getName() const{

        using namespace std;
        string name = this->full_path.getCString();

        size_t found;


        found = name.find_last_of( "/\\" );

        if( found ){

            name = name.substr( found + 1 );
            return Utf8String( name );

        }

        return this->full_path;

    }



    /**
     * Gets the current working directory as a string.
     *
     */
    Utf8String Directory::getWorkingPath(){

        // Credit to Pete Kirkham - http://stackoverflow.com/a/2203853/278976

        char temp [ PATH_MAX ];

        if( getcwd(temp, PATH_MAX) == 0 ){
            return Utf8String( temp );
        }

        int error = errno;

        switch( error ){
            // EINVAL can't happen - size argument > 0

            // PATH_MAX includes the terminating null so ERANGE should not be returned

            case EACCES:
                throw new Exception( "Permission to read or search a component of the filename was denied." );

            case ENOMEM:
                throw new Exception( "Insufficient storage error from getcwd." );

            case EFAULT:
                throw new Exception( "buf points to a bad address." );

            default: {
                throw new Exception( "Unrecognised error from getcwd." );
            }
        };

    }


}