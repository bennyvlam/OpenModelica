
long unsigned int szMemoryUsed = 0;

#if defined(_MSC_VER)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Modelica_3_Lexer.h>
#include <ModelicaParser.h>

void Main_5finit(void)
{
 static int done = 0;
 if( done ) return;
 done = 1;
 RML_5finit(); // this is needed for initialization.
}

int parseFile(char* fileName)
{
  pANTLR3_UINT8         fName;
  pANTLR3_INPUT_STREAM  input;
  pModelica_3_Lexer     lxr;
  pANTLR3_COMMON_TOKEN_STREAM tstream;
  pModelicaParser       psr;
  
  fprintf(stderr, "Parsing %s\n", fileName); fflush(stderr);

  fName  = (pANTLR3_UINT8)fileName;
  input  = antlr3AsciiFileStreamNew(fName);
  if ( input == NULL ) { fprintf(stderr, "Unable to open file %s\n", (char *)fName); exit(ANTLR3_ERR_NOMEM); }

  lxr      = Modelica_3_LexerNew(input);
  if (lxr == NULL ) { fprintf(stderr, "Unable to create the lexer due to malloc() failure1\n"); exit(ANTLR3_ERR_NOMEM); }

  tstream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lxr));
  if (tstream == NULL) { fprintf(stderr, "Out of memory trying to allocate token stream\n"); exit(ANTLR3_ERR_NOMEM); }
  tstream->channel = ANTLR3_TOKEN_DEFAULT_CHANNEL;
  tstream->discardOffChannel = ANTLR3_TRUE;
  tstream->discardOffChannelToks(tstream, ANTLR3_TRUE);

  // Finally, now that we have our lexer constructed, create the parser
  psr      = ModelicaParserNew(tstream);  // ModelicaParserNew is generated by ANTLR3

  if (tstream == NULL) { fprintf(stderr, "Out of memory trying to allocate parser\n"); exit(ANTLR3_ERR_NOMEM); }

  psr->stored_definition(psr);

  psr->free(psr);   psr = NULL;
  tstream->free(tstream); tstream = NULL;
  lxr->free(lxr);   lxr = NULL;
  input->close(input);    input = NULL;

  return 0;
}

#if defined(_MSC_VER)
int hasMoFiles(char* directory)
{
  WIN32_FIND_DATA FileData;
  BOOL more = TRUE;
  char pattern[3000];
  HANDLE sh;

  sprintf(pattern, "%s\\*.mo", directory);

  sh = FindFirstFile(pattern, &FileData);
  if (sh != INVALID_HANDLE_VALUE) 
  {
    FindClose(sh);
    return 1;
  }
  FindClose(sh);
  return 0;
}

int parseMoFilesInDirectory(char* directory)
{
  WIN32_FIND_DATA FileData;
  BOOL more = TRUE;
  char pattern[3000];
  char fileName[3000];
  HANDLE sh;
  /*
  HANDLE parseThreadHandle;
  DWORD parseThreadId;
  semaphore = CreateSemaphore( 
  NULL, // default security attributes
  5,   // initial count
  5,   // maximum count
  NULL);// unnamed semaphore
  */
  sprintf(pattern, "%s\\*.mo", directory);

  sh = FindFirstFile(pattern, &FileData);
  if (sh != INVALID_HANDLE_VALUE) 
  {
    while(more)
    {
      sprintf(fileName, "%s/%s", directory, FileData.cFileName);
      // Start thread that parses a file.
      // parseThreadHandle = CreateThread(NULL, 0, parseFile, (void*)strdup(fileName), 0, &parseThreadId);
      parseFile(fileName);
      more = FindNextFile(sh, &FileData);
    }
    if (sh != INVALID_HANDLE_VALUE) FindClose(sh);
  }

  // CloseHandle(semaphore);

  return 0;
}

int recurseDirectories(char* directory)
{
  WIN32_FIND_DATA FileData;
  BOOL more = TRUE;
  char pattern[3000];
  char directoryName[3000];
  HANDLE sh;

  sprintf(pattern, "%s\\*", directory);

  sh = FindFirstFile(pattern, &FileData);
  if (sh != INVALID_HANDLE_VALUE) 
  {
    while(more)
    {
      if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && 
  strcmp(FileData.cFileName,".") != 0 &&  
  strcmp(FileData.cFileName,"..") != 0)
      {
  sprintf(directoryName, "%s/%s", directory, FileData.cFileName);
  if (hasMoFiles(directoryName))
  {
    fprintf(stderr, "Parsing %s\n",directoryName); fflush(stderr);
    recurseDirectories(directoryName);
    parseMoFilesInDirectory(directoryName);
  }
  else
  {
    recurseDirectories(directoryName);
  }
      }
      more = FindNextFile(sh, &FileData);
    }
    if (sh != INVALID_HANDLE_VALUE) FindClose(sh);
  }
  return 0;
}
#endif

int main(int argc, char** argv)
{
  Main_5finit();
  double memoryUsed=0.0,memoryUsedKB=0.0,memoryUsedMB=0.0,memoryUsedGB=0.0;

  if ( argc < 3 ) fprintf(stderr, "Usage: %s [-f Model.mo|-d directory]\n",argv[0]),exit(1);

#if defined(_MSC_VER)
  if (strcmp(argv[1],"-d") == 0)
  {
    parseMoFilesInDirectory(argv[2]);
    recurseDirectories(argv[2]);
    fprintf(stderr, "No more .mo files!\n");
  }
#endif

  if (strcmp(argv[1],"-f") == 0)
  {
    parseFile(argv[2]);
  }

  memoryUsed = (double)szMemoryUsed;
  memoryUsedKB = memoryUsed/1024.0;   /* in K */
  memoryUsedMB = memoryUsedKB/1024.0; /* in M */
  memoryUsedGB = memoryUsedMB/1024.0; /* in G */

  fprintf(stderr, "Total memory allocated during parse: %.3f Mb ~ %.3f Gb\n", memoryUsedMB, memoryUsedGB); fflush(stderr);
  return 0;
}

