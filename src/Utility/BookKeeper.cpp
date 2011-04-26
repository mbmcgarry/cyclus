// BookKeeper.cpp
// Implements the BookKeeper class
#include "BookKeeper.h"
#include "Timer.h"
#include "boost/multi_array.hpp"

#include "hdf5.h"
#include "H5Cpp.h"
#include "H5Exception.h"
#include "GenException.h"

BookKeeper* BookKeeper::_instance = 0;

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BookKeeper* BookKeeper::Instance()
{
	// If we haven't created a BookKeeper yet, create and return it.
	if (0 == _instance)
		_instance = new BookKeeper();
	
	return _instance;
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
BookKeeper::BookKeeper() 
{
  dbIsOpen = false;
  dbExists = false;
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::createDB(){
  createDB("cyclus.h5");
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::createDB(string name){
  dbName = name;

  try{
    // create database. If it already exits, H5F_ACC_TRUNC erases all 
    // data previously stored in the file.
    myDB = new H5File( name , H5F_ACC_TRUNC );
    dbIsOpen = true; // use the H5 function for this.. !!! KDHFLAG
    dbExists = true;

    // create groups for the input and output data, respectively
    Group* outGroup = new Group( myDB->createGroup("/output"));
    Group* inGroup = new Group( myDB->createGroup("/input"));

    delete outGroup;
    delete inGroup;
  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
DataSet BookKeeper::createDataSet(hsize_t rank, hsize_t* dims, DataType type, string dsName){
  DataSet dataset;
  try{
    // create the dataspace from rank and dimension information
    DataSpace* dataspace = new DataSpace(rank, dims);

    // create a dataset to match the dataspace
    dataset = this->getDB()->createDataSet(dsName, type, *dataspace) ; 

  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
  return dataset;
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
H5File* BookKeeper::getDB()
{
	return myDB;
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::openDB()
{
  if(dbIsOpen==false){
    //If the database is already open, throw an exception; the caller probably 
	  // doesn't realize this.
    try{ 
      myDB = new H5File(dbName, H5F_ACC_RDWR);
	    dbIsOpen = true;
    }
    catch( FileIException error )
    {
      error.printError();
    }
  };
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::closeDB()
{
  try{
    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    Exception::dontPrint();
    if(dbIsOpen==true){
      this->getDB()->close();
      dbIsOpen = false;
    }
    else
      throw GenException("Tried to close a database that was not open."); 
  }
  catch( FileIException error )
  {
    error.printError();
  } 
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
bool BookKeeper::isGroup(string grp)
{
  bool toRet = true;
//  string testname;
//  int iter, subiter;
//  hid_t idx;
//  deque<hid_t> subgroups; 
//  CommonFG* obj;
//
//  obj = this->getDB();
//  hsize_t nObs = obj->getNumObjs();
//  subgroups[0]= 0 ;
//  bool toRet = false;
//  subiter = 0;
//
//  while( toRet==false && subgroups.size()!=0){
//    iter = 0; 
//    nObs = obj->getNumObjs();
//    while( iter < nObs){
//      testname = obj->getObjnameByIdx(iter);
//      if(obj->getObjTypeByIdx(iter) == H5G_GROUP){
//        subgroups.push_back( iter );
//      };
//      if( testname == grp  && obj->getObjTypeByIdx(iter) == H5G_GROUP){
//        toRet = true;
//      };
//      iter++;
//    };
//    delete obj;
//    hid_t idx = subgroups.pop_front();
//    obj = new Group(idx);
//  };
//
  return toRet;
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeModelList(ModelType type){

  // define some useful variables.
  const H5std_string ID_memb = "ID";
  const H5std_string name_memb = "name";
  const H5std_string modelImpl_memb = "modelImpl";
  const H5std_string output_name = "output";
  string subgroup_name;
  string dataset_name;

  // prepare the function pointer
  Model* (Logician::*ptr2getModel)(int) = NULL;

  // parse the cases.
  int numModels;
  switch( type ) {
    case region:
      numModels = LI->getNumRegions();
      subgroup_name = "regions";
      dataset_name = "regionList"; break;
      ptr2getModel = &Logician::getRegionByID;
    case inst:
      numModels = LI->getNumInsts();
      subgroup_name = "insts";
      dataset_name = "instList"; break;
      ptr2getModel = &Logician::getInstByID;
    case facility:
      numModels = LI->getNumFacilities();
      subgroup_name = "facilities";
      dataset_name = "facList"; break;
      ptr2getModel = &Logician::getFacilityByID;
    case market:
      numModels = LI->getNumMarkets();
      subgroup_name = "markets";
      dataset_name = "marketList"; break;
      ptr2getModel = &Logician::getMarketByID;
    case converter:
      numModels = LI->getNumConverters();
      subgroup_name = "converters";
      dataset_name = "converterList"; break;
      ptr2getModel = &Logician::getConverterByID;
  };

  // create an array of the model structs
  model_t modelList[numModels];
  for (int i=0; i<numModels; i++){
    modelList[i].ID = i;
    Model* theModel = (LI->*ptr2getModel)(i);
    strcpy(modelList[i].modelImpl, theModel->getModelImpl().c_str());
    strcpy(modelList[i].name, theModel->getName().c_str()); 
  };

  try{
    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    Exception::dontPrint();
    
    // Open the file and the dataset.
    this->openDB();

    // describe the data in an hdf5-y way
    hsize_t dim[] = {1,numModels};
    int rank = 1;
    Group* outputgroup;
    outputgroup = new Group(myDB->openGroup(output_name));
    Group* subgroup;
    subgroup = new Group(outputgroup->createGroup("/"+output_name + "/" + subgroup_name));
    DataSpace* dataspace;
    dataspace = new DataSpace( rank, dim );

    //create a variable length string types
    StrType vls_type(0, H5T_VARIABLE); 
    DataType strtype = DataType(vls_type);

   
    // Create a datatype for models based on the struct
    CompType mtype( sizeof(model_t) );
    mtype.insertMember( name_memb, HOFFSET(model_t, name), strtype);
    mtype.insertMember( modelImpl_memb, HOFFSET(model_t, modelImpl), strtype);
    mtype.insertMember( ID_memb, HOFFSET(model_t, ID), PredType::NATIVE_INT); 

    DataSet* dataset;
    dataset = new DataSet(subgroup->createDataSet( dataset_name , mtype , *dataspace ));

    // write it, finally 
    dataset->write( modelList , mtype );

    delete outputgroup;
    delete subgroup;
    delete dataspace;
    delete dataset;
  }
  // catch failure caused by the H5File operations
  catch( FileIException error )
  {
     error.printError();
  }
  // catch failure caused by the Group operations
  catch( GroupIException error )
  {
     error.printError();
  }
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
  // catch failure caused by the DataType operations
  catch( DataTypeIException error )
  {
     error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::prepareSpaces(string dsname, DataType type, DataSpace &memspace, 
    DataSpace &filespace, DataSet &dataset){
    
    // define the dataset to match the dataspace
    dataset = this->getDB()->createDataSet(dsname, type, memspace) ; 

    // define the file dataspace to match the dataset
    filespace = dataset.getSpace();
  
    // select the whole dataset as the memory dataspace
    memspace.selectAll();
    
    // select the whole dataset as the memory dataspace
    filespace.selectAll();

};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeData(intData1d data, string dsname){ 
  try {
    hsize_t nrows = data.size(); 
  
    hsize_t dims[1];
    dims[0]=nrows;
    hsize_t rank = 1;

    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims);
    // create a file dataspace 
    DataSpace filespace;
    // create a dataset
    DataSet dataset;
    // name the type
    DataType type = DataType(PredType::NATIVE_INT);
    // prepare the spaces
    this->prepareSpaces(dsname, type, memspace, filespace, dataset);
    
    // the data needs to be an array - is this memcpy ugly?
    double dat_array[nrows];
    memcpy( dat_array, &data[0], sizeof( int ) * data.size() );
  
    // write it
    dataset.write(dat_array, type, memspace, filespace );

  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( DataSpaceIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeData(intData2d data, string dsname){
  try {
    hsize_t nrows = data.size(); 
    hsize_t ncols = data[0].size(); 
    
    hsize_t dims[] = {nrows, ncols};
    hsize_t rank = 2;
    
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
    // create a file dataspace 
    DataSpace filespace;
    // create a dataset
    DataSet dataset;
    // name the type
    DataType type = DataType(PredType::NATIVE_INT);
    // prepare the spaces
    this->prepareSpaces(dsname, type, memspace, filespace, dataset);
    
    // the data needs to be an array - is this memcpy ugly?
    int dat_array[nrows][ncols];
    for(int row=0; row<nrows; row++){
      memcpy( dat_array[row], &data[row][0], sizeof( int ) * ncols );
    };

    // write it
    dataset.write(dat_array, type, memspace, filespace );
  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSpaceIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeData(intData3d data, string dsname){

  try {
    hsize_t nrows = data.size(); 
    hsize_t ncols = data[0].size(); 
    hsize_t nlayers = data[0][0].size(); 
    
    hsize_t dims[] = {nrows, ncols,nlayers};
    hsize_t rank = 3;
    
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
    // create a file dataspace 
    DataSpace filespace;
    // create a dataset
    DataSet dataset;
    // name the type
    DataType type = DataType(PredType::NATIVE_INT);
    // prepare the spaces
    this->prepareSpaces(dsname, type, memspace, filespace, dataset);
    
    // the data needs to be an array - is this memcpy ugly?
    int dat_array[nrows][ncols][nlayers];
    for(int row=0; row<nrows; row++){
      for(int col=0; col<ncols; col++){
        memcpy( dat_array[row][col], &data[row][col][0], sizeof( int ) * nlayers);
      };
    };

    // write it
    dataset.write(dat_array, type, memspace, filespace );
  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSpaceIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeData(dblData1d data, string dsname){
  try {
    hsize_t nrows = data.size(); 
  
    hsize_t dims[1];
    dims[0]=nrows;
    hsize_t rank = 1;

    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
    // create a file dataspace 
    DataSpace filespace;
    // create a dataset
    DataSet dataset;
    // name the type
    DataType type = DataType(PredType::IEEE_F64LE);
    // prepare the spaces
    this->prepareSpaces(dsname, type, memspace, filespace, dataset);
  
    // the data needs to be an array - is this memcpy ugly?
    double dat_array[nrows];
    memcpy( dat_array, &data[0], sizeof( double ) * data.size() );
  
    // write it
    dataset.write(dat_array, type, memspace, filespace );
  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeData(dblData2d data, string dsname){
  try {
    hsize_t nrows = data.size(); 
    hsize_t ncols = data[0].size(); 
    
    hsize_t dims[] = {nrows, ncols};
    hsize_t rank = 2;

    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
    // create a file dataspace 
    DataSpace filespace;
    // create a dataset
    DataSet dataset;
    // name the type
    DataType type = DataType(PredType::IEEE_F64LE);
    // prepare the spaces
    this->prepareSpaces(dsname, type, memspace, filespace, dataset);

    // the data needs to be an array - is this memcpy ugly?
    double dat_array[nrows][ncols];
    for(int row=0; row<nrows; row++){
      memcpy( dat_array[row], &data[row][0], sizeof( double ) * ncols );
    };

    // write it
    dataset.write(dat_array, type, memspace, filespace );
  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSpaceIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeData(dblData3d data, string dsname){
  try {
    hsize_t nrows = data.size(); 
    hsize_t ncols = data[0].size(); 
    hsize_t nlayers = data[0][0].size(); 
    
    hsize_t dims[] = {nrows, ncols,nlayers};
    hsize_t rank = 3;

    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
    // create a file dataspace 
    DataSpace filespace;
    // create a dataset
    DataSet dataset;
    // name the type
    DataType type = DataType(PredType::IEEE_F64LE);
    // prepare the spaces
    this->prepareSpaces(dsname, type, memspace, filespace, dataset);
    
    // the data needs to be an array - is this memcpy ugly?
    double dat_array[nrows][ncols][nlayers];
    for(int row=0; row<nrows; row++){
      for(int col=0; col<ncols; col++){
        memcpy( dat_array[row][col], &data[row][col][0], sizeof( double ) * nlayers);
      };
    };

    // write it
    dataset.write(dat_array, type, memspace, filespace );
  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSpaceIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeData(strData1d data, string dsname){
  try {
    hsize_t nrows = data.size(); 
  
    hsize_t dims[1];
    dims[0]=nrows;
    hsize_t rank = 1;
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
    // create a file dataspace 
    DataSpace filespace;
    // create a dataset
    DataSet dataset;
    //create a variable length string types
    StrType vls_type(0, H5T_VARIABLE); 
    DataType type = DataType(vls_type);
    // prepare the spaces
    this->prepareSpaces(dsname, type, memspace, filespace, dataset);

    // the data needs to be an array
    string dat_array[nrows];
    dat_array;
    for(int row=0; row < nrows;row++){
      dat_array[row]=data[row];
    };
  
    // write it
    dataset.write(dat_array, type, memspace, filespace );
  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeData(strData2d data, string dsname){
  try {
    hsize_t nrows = data.size(); 
    hsize_t ncols = data[0].size(); 
    
    hsize_t dims[] = {nrows, ncols};
    hsize_t rank = 2;

    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
    // create a file dataspace 
    DataSpace filespace;
    // create a dataset
    DataSet dataset;
    //create a variable length string types
    StrType vls_type(0, H5T_VARIABLE); 
    DataType type = DataType(vls_type);
    // prepare the spaces
    this->prepareSpaces(dsname, type, memspace, filespace, dataset);
    
    // the data needs to be an array
    string dat_array[nrows][ncols];
    for(int row=0; row<nrows; row++){
      for(int col=0; col<ncols; col++){
        dat_array[row][col]=data[row][col];
      };
    };

    // write it
    dataset.write(dat_array, type, memspace, filespace );

  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSpaceIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::writeData(strData3d data, string dsname){
  try {
    hsize_t nrows = data.size(); 
    hsize_t ncols = data[0].size(); 
    hsize_t nlayers = data[0][0].size(); 
    
    hsize_t dims[] = {nrows, ncols,nlayers};
    hsize_t rank = 3;

    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
    // create a file dataspace 
    DataSpace filespace;
    // create a dataset
    DataSet dataset;
    //create a variable length string types
    StrType vls_type(0, H5T_VARIABLE); 
    DataType type = DataType(vls_type);
    // prepare the spaces
    this->prepareSpaces(dsname, type, memspace, filespace, dataset);
    
    // the data needs to be an array 
    string dat_array[nrows][ncols][nlayers];
    for(int row=0; row<nrows; row++){
      for(int col=0; col<ncols; col++){
        for(int layer=0; layer<nlayers;layer++){
          dat_array[row][col][layer]=data[row][col][layer];
        };
      };
    };

    // write it
    dataset.write(dat_array, type, memspace, filespace );
  }
  catch( FileIException error )
  {
    error.printError();
  }
  catch( DataSpaceIException error )
  {
    error.printError();
  }
  catch( DataSetIException error )
  {
    error.printError();
  }
  catch( GroupIException error )
  {
    error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::readData(string dsname, intData1d& out_data){

  try{
    // turn off auto printing and deal with exceptions at the end
    Exception::dontPrint();
    // get the dataset open
    DataSet dataset = myDB->openDataSet(dsname);
    // get the class of the datatype used in the dataset
    H5T_class_t type_class = dataset.getTypeClass(); 

    // double check that its an integer
    IntType inttype;
    if( type_class == H5T_INTEGER ) 
    {
      // oh good, it's an integer. Now figure out what kind.
      inttype = dataset.getIntType();
    }
    else{
      throw GenException("The dataset " + dsname + " is not of integer type");
    }
  
    // get the file dataspace
    DataSpace filespace = dataset.getSpace();
  
    // find the rank
    int rank = filespace.getSimpleExtentNdims();
  
    // find what the dataspace dimensions are
    hsize_t dims[2];
    int ndims = filespace.getSimpleExtentDims(dims, NULL);
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
  
    // select everything in each dataspace
    memspace.selectAll();
    filespace.selectAll();
  
    int out_array[dims[0]];
    for (int i=0; i<dims[0]; i++){
      out_array[i]=1;
    };
  
    dataset.read( out_array, inttype, memspace , filespace );
    
    out_data.assign(out_array, out_array + sizeof(out_array));
  } 
  catch( FileIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     error.printError();
  }

};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::readData(string dsname, intData2d& out_data){

  try{
    // turn off auto printing and deal with exceptions at the end
    Exception::dontPrint();
    // get the dataset open
    DataSet dataset = myDB->openDataSet(dsname);
    // get the class of the datatype used in the dataset
    H5T_class_t type_class = dataset.getTypeClass(); 
    // double check that its an integer
    IntType inttype;
    if( type_class == H5T_INTEGER ) 
    {
      // oh good, it's an integer. Now figure out what kind.
      inttype = dataset.getIntType();
    }
    else{
      throw GenException("The dataset " + dsname + " is not of integer type");
    }
  
    // get the file dataspace
    DataSpace filespace = dataset.getSpace();
  
    // find the rank
    int rank = filespace.getSimpleExtentNdims();
  
    // find what the dataspace dimensions are
    hsize_t dims[2];
    int ndims = filespace.getSimpleExtentDims(dims, NULL);
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
  
    // select everything in each dataspace
    memspace.selectAll();
    filespace.selectAll();
  
    int out_array[dims[0]][dims[1]];
    for (int i=0; i<dims[0]; i++){
      for (int j=0; j<dims[1]; j++){
        out_array[i][j]=1;
      };
    };
  
    dataset.read( out_array, inttype, memspace , filespace );

    for(int row=0; row<dims[0]; row++){
      for(int col=0; col<dims[1]; col++){
        out_data[row][col] = out_array[row][col];
      };
    };

    
  } 
  catch( FileIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     error.printError();
  }

};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::readData(string dsname, intData3d& out_data){

  try{
    // turn off auto printing and deal with exceptions at the end
    Exception::dontPrint();
    // get the dataset open
    DataSet dataset = myDB->openDataSet(dsname);
    // get the class of the datatype used in the dataset
    H5T_class_t type_class = dataset.getTypeClass(); 
    // double check that its an integer
    IntType inttype;
    if( type_class == H5T_INTEGER ) 
    {
      // oh good, it's an integer. Now figure out what kind.
      inttype = dataset.getIntType();
    }
    else{
      throw GenException("The dataset " + dsname + " is not of integer type");
    }
  
    // get the file dataspace
    DataSpace filespace = dataset.getSpace();
  
    // find the rank
    int rank = filespace.getSimpleExtentNdims();
  
    // find what the dataspace dimensions are
    hsize_t dims[3];
    int ndims = filespace.getSimpleExtentDims(dims, NULL);
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
  
    // select everything in each dataspace
    memspace.selectAll();
    filespace.selectAll();
  
    int out_array[dims[0]][dims[1]][dims[2]];
    for (int i=0; i<dims[0]; i++){
      for (int j=0; j<dims[1]; j++){
        for (int k=0; k<dims[2]; k++){
          out_array[i][j][k]=1;
        };
      };
    };
  
    dataset.read( out_array, inttype, memspace , filespace );

    for(int row=0; row<dims[0]; row++){
      for(int col=0; col<dims[1]; col++){
        for(int layer=0; layer<dims[2];layer++){
          out_data[row][col][layer] = out_array[row][col][layer];
        };
      };
    };
  } 
  catch( FileIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::readData(string dsname, dblData1d& out_data){
  try{
    // turn off auto printing and deal with exceptions at the end
    Exception::dontPrint();
    // get the dataset open
    DataSet dataset = myDB->openDataSet(dsname);
    // get the class of the datatype used in the dataset
    H5T_class_t type_class = dataset.getTypeClass(); 

    // double check that its a double
    FloatType dbltype;
    if( type_class == H5T_FLOAT ) 
    {
      // oh good, it's a double. Now figure out what kind.
      dbltype = dataset.getFloatType();
    }
    else{
      throw GenException("The dataset " + dsname + " is not of float type");
    }
  
    // get the file dataspace
    DataSpace filespace = dataset.getSpace();
  
    // find the rank
    int rank = filespace.getSimpleExtentNdims();
  
    // find what the dataspace dimensions are
    hsize_t dims[2];
    int ndims = filespace.getSimpleExtentDims(dims, NULL);
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
  
    // select everything in each dataspace
    memspace.selectAll();
    filespace.selectAll();
  
    double out_array[dims[0]];
    for (int i=0; i<dims[0]; i++){
      out_array[i]=1;
    };
  
    dataset.read( out_array, dbltype, memspace , filespace );

    for(int row=0; row<dims[0]; row++){
      out_data[row]= out_array[row];
    };
    
    out_data.assign(out_array, out_array + sizeof(out_array));
  } 
  catch( FileIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     error.printError();
  }

};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::readData(string dsname, dblData2d& out_data){
  try{
    // turn off auto printing and deal with exceptions at the end
    Exception::dontPrint();
    // get the dataset open
    DataSet dataset = myDB->openDataSet(dsname);
    // get the class of the datatype used in the dataset
    H5T_class_t type_class = dataset.getTypeClass(); 
    // double check that its a double
    FloatType dbltype;
    if( type_class == H5T_FLOAT ) 
    {
      // oh good, it's a double. Now figure out what kind.
      dbltype = dataset.getFloatType();
    }
    else{
      throw GenException("The dataset " + dsname + " is not of float type");
    }
  
    // get the file dataspace
    DataSpace filespace = dataset.getSpace();
  
    // find the rank
    int rank = filespace.getSimpleExtentNdims();
  
    // find what the dataspace dimensions are
    hsize_t dims[2];
    int ndims = filespace.getSimpleExtentDims(dims, NULL);
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
  
    // select everything in each dataspace
    memspace.selectAll();
    filespace.selectAll();
  
    double out_array[dims[0]][dims[1]];
    for (int i=0; i<dims[0]; i++){
      for (int j=0; j<dims[1]; j++){
        out_array[i][j]=1.0;
      };
    };
  
    dataset.read( out_array, dbltype, memspace , filespace );

    for(int row=0; row<dims[0]; row++){
      for(int col=0; col<dims[1]; col++){
        out_data[row][col] = out_array[row][col];
      };
    };
    

  } 
  catch( FileIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     error.printError();
  }

};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::readData(string dsname, dblData3d& out_data){

  try{
    // turn off auto printing and deal with exceptions at the end
    Exception::dontPrint();
    // get the dataset open
    DataSet dataset = myDB->openDataSet(dsname);
    // get the class of the datatype used in the dataset
    H5T_class_t type_class = dataset.getTypeClass(); 
    // double check that its a double
    FloatType dbltype;
    if( type_class == H5T_FLOAT ) 
    {
      // oh good, it's a double. Now figure out what kind.
      dbltype = dataset.getFloatType();
    }
    else{
      throw GenException("The dataset " + dsname + " is not of double type");
    }
  
    // get the file dataspace
    DataSpace filespace = dataset.getSpace();
  
    // find the rank
    int rank = filespace.getSimpleExtentNdims();
  
    // find what the dataspace dimensions are
    hsize_t dims[3];
    int ndims = filespace.getSimpleExtentDims(dims, NULL);
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
  
    // select everything in each dataspace
    memspace.selectAll();
    filespace.selectAll();
  
    double out_array[dims[0]][dims[1]][dims[2]];
    for (int i=0; i<dims[0]; i++){
      for (int j=0; j<dims[1]; j++){
        for (int k=0; k<dims[2]; k++){
          out_array[i][j][k]=1.0;
        };
      };
    };
  
    dataset.read( out_array, dbltype, memspace , filespace );

    for(int row=0; row<dims[0]; row++){
      for(int col=0; col<dims[1]; col++){
        for(int layer=0; layer<dims[2];layer++){
          out_data[row][col][layer] = out_array[row][col][layer];
        };
      };
    };

  } 
  catch( FileIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::readData(string dsname, strData1d& out_data){
  try{
    // turn off auto printing and deal with exceptions at the end
    Exception::dontPrint();
    // get the dataset open
    DataSet dataset = myDB->openDataSet(dsname);
    // get the class of the datatype used in the dataset
    H5T_class_t type_class = dataset.getTypeClass(); 

    // double check that its a string
    StrType strtype;
    if( type_class == H5T_STRING ) 
    {
      // oh good, it's a string. Now figure out what kind.
      strtype = dataset.getStrType();
    }
    else{
      throw GenException("The dataset " + dsname + " is not of string type");
    }
  
    // get the file dataspace
    DataSpace filespace = dataset.getSpace();
  
    // find the rank
    int rank = filespace.getSimpleExtentNdims();
  
    // find what the dataspace dimensions are
    hsize_t dims[2];
    int ndims = filespace.getSimpleExtentDims(dims, NULL);
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
  
    // select everything in each dataspace
    memspace.selectAll();
    filespace.selectAll();
  
    string out_array[dims[0]];
    for (int i=0; i<dims[0]; i++){
      out_array[i]="";
    };
  
    dataset.read( out_array, strtype, memspace , filespace );

    for(int row=0; row<dims[0]; row++){
      out_data[row] = out_array[row];
    };

  } 
  catch( FileIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::readData(string dsname, strData2d& out_data){
  try{
    // turn off auto printing and deal with exceptions at the end
    Exception::dontPrint();
    // get the dataset open
    DataSet dataset = myDB->openDataSet(dsname);
    // get the class of the datatype used in the dataset
    H5T_class_t type_class = dataset.getTypeClass(); 
    // double check that its a string
    StrType strtype;
    if( type_class == H5T_STRING ) 
    {
      // oh good, it's a string. Now figure out what kind.
      strtype = dataset.getStrType();
    }
    else{
      throw GenException("The dataset " + dsname + " is not of string type");
    }
  
    // get the file dataspace
    DataSpace filespace = dataset.getSpace();
  
    // find the rank
    int rank = filespace.getSimpleExtentNdims();
  
    // find what the dataspace dimensions are
    hsize_t dims[2];
    int ndims = filespace.getSimpleExtentDims(dims, NULL);
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
  
    // select everything in each dataspace
    memspace.selectAll();
    filespace.selectAll();
  
    string out_array[dims[0]][dims[1]];
    for (int i=0; i<dims[0]; i++){
      for (int j=0; j<dims[1]; j++){
        out_array[i][j]="";
      };
    };
  
    dataset.read( out_array, strtype, memspace , filespace );

    for(int row=0; row<dims[0]; row++){
      for(int col=0; col<dims[1]; col++){
        out_data[row][col] = out_array[row][col];
      };
    };

  } 
  catch( FileIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     error.printError();
  }
};

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
void BookKeeper::readData(string dsname, strData3d& out_data){

  try{
    // turn off auto printing and deal with exceptions at the end
    Exception::dontPrint();
    // get the dataset open
    DataSet dataset = myDB->openDataSet(dsname);
    // get the class of the datatype used in the dataset
    H5T_class_t type_class = dataset.getTypeClass(); 
    // double check that its an string
    StrType  strtype;
    if( type_class == H5T_STRING ) 
    {
      // oh good, it's a string. Now figure out what kind.
      strtype = dataset.getStrType();
    }
    else{
      throw GenException("The dataset " + dsname + " is not of string type");
    }
  
    // get the file dataspace
    DataSpace filespace = dataset.getSpace();
  
    // find the rank
    int rank = filespace.getSimpleExtentNdims();
  
    // find what the dataspace dimensions are
    hsize_t dims[3];
    int ndims = filespace.getSimpleExtentDims(dims, NULL);
  
    // create a memory dataspace
    DataSpace memspace = DataSpace( rank, dims );
  
    // select everything in each dataspace
    memspace.selectAll();
    filespace.selectAll();
  
    string out_array[dims[0]][dims[1]][dims[2]];
    for (int i=0; i<dims[0]; i++){
      for (int j=0; j<dims[1]; j++){
        for (int k=0; k<dims[2]; k++){
          out_array[i][j][k]="";
        };
      };
    };
  
    dataset.read( out_array, strtype, memspace , filespace );

    for(int row=0; row<dims[0]; row++){
      for(int col=0; col<dims[1]; col++){
        for(int layer=0; layer<dims[2];layer++){
          out_data[row][col][layer] = out_array[row][col][layer];
        };
      };
    };
  } 
  catch( FileIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSet operations
  catch( DataSetIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataSpaceIException error )
  {
     error.printError();
  }
 
  // catch failure caused by the DataSpace operations
  catch( DataTypeIException error )
  {
     error.printError();
  }
};