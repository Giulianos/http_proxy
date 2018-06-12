#!/bin/bash

MODULE_DIR_NAME=$2
MODULE_DIR=src/$2
MODULE_NAME=$1

USAGE='./new_module.sh [module-name] [module-dir-name]'
EXAMPLE_USAGE='./new_module.sh MyModule my_module'

if [ "$MODULE_NAME" = "" ];
then
  echo 'Module name not specified!'
  echo 'Usage: '$USAGE
  echo ' example: '$EXAMPLE_USAGE
  exit 1
fi

if [ "$MODULE_DIR_NAME" = "" ];
then
  echo 'Module directory not specified!'
  echo 'Usage: '$USAGE
  echo ' example: '$EXAMPLE_USAGE
  exit 1
fi

# Move into src
cd src

echo 'Creating module directory...'
mkdir $MODULE_DIR_NAME

if [ $? -ne 0 ];
then
  echo 'Module already exists! Exiting...'
  exit 1
fi

# Move into the module directory
cd $MODULE_DIR_NAME

#Create CMakeLists.txt
echo 'Creating module CMakeLists.txt...'
echo 'add_library('$MODULE_NAME' sample_source.c)' >> CMakeLists.txt
echo '# Dependecies' >> CMakeLists.txt
echo '# target_link_libraries('$MODULE_NAME' DependencyName)' >> CMakeLists.txt
echo 'Creating sample source file (sample_source.c)...'
touch sample_source.c

# Move into src
cd ..

#Add module directory to src's CMakeLists.txt
echo 'Adding module directory to admin_client CMakeLists.txt'
sed -i '/# Subdirectories/a add_subdirectory\('$MODULE_DIR_NAME'\)' CMakeLists.txt

#Add module to list of libraries to link
echo 'Adding module as admin_client dependency'
sed -i '/# Libraries/a target_link_libraries\(admin_client '$MODULE_NAME'\)' CMakeLists.txt

#Create template for module contract
echo 'Creating template for module contract...'
cd include
mkdir $MODULE_DIR_NAME
cd $MODULE_DIR_NAME
MODULE_CONTRACT_DEFINE=${MODULE_NAME^^}'_H'
echo '#ifndef '$MODULE_CONTRACT_DEFINE >> $MODULE_DIR_NAME.h
echo '#define '$MODULE_CONTRACT_DEFINE >> $MODULE_DIR_NAME.h
echo '  ' >>  $MODULE_DIR_NAME.h
echo '#endif' >> $MODULE_DIR_NAME.h

echo 'Module created successfully!'
echo 'Remember:'
echo ' - .h files (contract of the module) goes into src/include'
echo ' - .c files (source of the module) goes into '$MODULE_DIR'/ and should be added to the CMakeLists.txt'
echo ' - dependencies for the module should be added in '$MODULE_DIR'/CMakeLists.txt'
