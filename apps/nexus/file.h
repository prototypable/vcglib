#ifndef NXS_FILE_H
#define NXS_FILE_H

//TODO move includes in cpp

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <string>

namespace nxs {

class File {
 public:

  File(): fp(NULL) {}
  ~File() { Close(); }   

  bool Create(const std::string &filename);
  bool Load(const std::string &filename);
  void Close();

  void Resize(unsigned int elem);

  void SetPosition(unsigned int chunk);
  void ReadBuffer(void *data, unsigned int size);
  void WriteBuffer(void *data, unsigned int size);

 protected:

#ifdef WIN32
   HANDLE fp;
#else
   FILE *fp;  
#endif
   unsigned int size;
};

}

#endif
