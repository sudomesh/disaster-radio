/*
 * This is library to serve files from an SD-card using Me No Dev's async webserver for the ESP8266.
 * See https://github.com/me-no-dev/ESPAsyncWebServer
 *
 * Adapted from an example by pim-borst, https://gist.github.com/pim-borst/17934bfd4454caea3ba4f74366c2135c
 *
 * Notes: 
 * In your main script you must specific #define FS_NO_GLOBALS, otherwise there is a namespace conflict between the SPIFFS and SD "File" type
 * Files and directories being served from the SD must follow an 8dot3 naming scheme, so no "html" files only "htm"
 */


bool SD_exists(SDClass &sd, String path) {
  // For some reason SD.exists(filename) reboots the ESP...
  // So we test by opening the file
  bool exists = false;
  File test = sd.open(path);
  if(test){
    test.close();
    exists = true;
  }
  return exists;
}

class AsyncSDFileResponse: public AsyncAbstractResponse {
  private:
    File _content;
    String _path;
    void _setContentType(const String& path);
    bool _sourceIsValid;
  public:
    AsyncSDFileResponse(SDClass &sd, const String& path, const String& contentType=String(), bool download=false);
    AsyncSDFileResponse(File content, const String& path, const String& contentType=String(), bool download=false);
    ~AsyncSDFileResponse();
    bool _sourceValid() const { return _sourceIsValid; } 
    virtual size_t _fillBuffer(uint8_t *buf, size_t maxLen) override;
};

AsyncSDFileResponse::~AsyncSDFileResponse(){
  if(_content)
    _content.close();
}

void AsyncSDFileResponse::_setContentType(const String& path){
  if (path.endsWith(".html")) _contentType = "text/html";
  else if (path.endsWith(".htm")) _contentType = "text/html";
  else if (path.endsWith(".css")) _contentType = "text/css";
  else if (path.endsWith(".json")) _contentType = "text/json";
  else if (path.endsWith(".js")) _contentType = "application/javascript";
  else if (path.endsWith(".png")) _contentType = "image/png";
  else if (path.endsWith(".gif")) _contentType = "image/gif";
  else if (path.endsWith(".jpg")) _contentType = "image/jpeg";
  else if (path.endsWith(".ico")) _contentType = "image/x-icon";
  else if (path.endsWith(".svg")) _contentType = "image/svg+xml";
  else if (path.endsWith(".eot")) _contentType = "font/eot";
  else if (path.endsWith(".woff")) _contentType = "font/woff";
  else if (path.endsWith(".woff2")) _contentType = "font/woff2";
  else if (path.endsWith(".ttf")) _contentType = "font/ttf";
  else if (path.endsWith(".xml")) _contentType = "text/xml";
  else if (path.endsWith(".pdf")) _contentType = "application/pdf";
  else if (path.endsWith(".zip")) _contentType = "application/zip";
  else if(path.endsWith(".gz")) _contentType = "application/x-gzip";
  else _contentType = "text/plain";
}

AsyncSDFileResponse::AsyncSDFileResponse(SDClass &sd, const String& path, const String& contentType, bool download){
  _code = 200;
  _path = path;
  
  if(!download && !SD_exists(sd, _path) && SD_exists(sd, _path+".gz")){
    _path = _path+".gz";
    addHeader("Content-Encoding", "gzip");
  }

  _content = sd.open(_path, FILE_READ);
  _contentLength = _content.size();
  _sourceIsValid = _content;

  if(contentType == "")
    _setContentType(path);
  else
    _contentType = contentType;

  int filenameStart = path.lastIndexOf('/') + 1;
  char buf[26+path.length()-filenameStart];
  char* filename = (char*)path.c_str() + filenameStart;

  if(download) {
    // set filename and force download
    snprintf(buf, sizeof (buf), "attachment; filename=\"%s\"", filename);
  } else {
    // set filename and force rendering
    snprintf(buf, sizeof (buf), "inline; filename=\"%s\"", filename);
  }
  addHeader("Content-Disposition", buf);

}

AsyncSDFileResponse::AsyncSDFileResponse(File content, const String& path, const String& contentType, bool download){
  _code = 200;
  _path = path;
  _content = content;
  _contentLength = _content.size();

  if(!download && String(_content.name()).endsWith(".gz") && !path.endsWith(".gz"))
    addHeader("Content-Encoding", "gzip");

  if(contentType == "")
    _setContentType(path);
  else
    _contentType = contentType;

  int filenameStart = path.lastIndexOf('/') + 1;
  char buf[26+path.length()-filenameStart];
  char* filename = (char*)path.c_str() + filenameStart;

  if(download) {
    snprintf(buf, sizeof (buf), "attachment; filename=\"%s\"", filename);
  } else {
    snprintf(buf, sizeof (buf), "inline; filename=\"%s\"", filename);
  }
  addHeader("Content-Disposition", buf);
}

size_t AsyncSDFileResponse::_fillBuffer(uint8_t *data, size_t len){
  _content.read(data, len);
  return len;
}

class AsyncStaticSDWebHandler: public AsyncWebHandler {
  private:
    bool _getFile(AsyncWebServerRequest *request);
    bool _fileExists(AsyncWebServerRequest *request, const String& path);
    uint8_t _countBits(const uint8_t value) const;
  protected:
    SDClass _sd;
    String _uri;
    String _path;
    String _default_file;
    String _cache_control;
    String _last_modified;
    bool _isDir;
    bool _gzipFirst;
    uint8_t _gzipStats;
  public:
    AsyncStaticSDWebHandler(const char* uri, SDClass& sd, const char* path, const char* cache_control = NULL);
    virtual bool canHandle(AsyncWebServerRequest *request) override final;
    virtual void handleRequest(AsyncWebServerRequest *request) override final;
    AsyncStaticSDWebHandler& setIsDir(bool isDir);
    AsyncStaticSDWebHandler& setDefaultFile(const char* filename);
    AsyncStaticSDWebHandler& setCacheControl(const char* cache_control);
    AsyncStaticSDWebHandler& setLastModified(const char* last_modified);
    AsyncStaticSDWebHandler& setLastModified(struct tm* last_modified);
  #ifdef ESP8266
    AsyncStaticSDWebHandler& setLastModified(time_t last_modified);
    AsyncStaticSDWebHandler& setLastModified(); //sets to current time. Make sure sntp is runing and time is updated
  #endif
};

AsyncStaticSDWebHandler::AsyncStaticSDWebHandler(const char* uri, SDClass& sd, const char* path, const char* cache_control)
  : _sd(sd), _uri(uri), _path(path), _default_file("index.htm"), _cache_control(cache_control), _last_modified("")
{
  // Ensure leading '/'
  if (_uri.length() == 0 || _uri[0] != '/') _uri = "/" + _uri;
  if (_path.length() == 0 || _path[0] != '/') _path = "/" + _path;

  // If path ends with '/' we assume a hint that this is a directory to improve performance.
  // However - if it does not end with '/' we, can't assume a file, path can still be a directory.
  _isDir = _path[_path.length()-1] == '/';

  // Remove the trailing '/' so we can handle default file
  // Notice that root will be "" not "/"
  if (_uri[_uri.length()-1] == '/') _uri = _uri.substring(0, _uri.length()-1);
  if (_path[_path.length()-1] == '/') _path = _path.substring(0, _path.length()-1);

  // Reset stats
  _gzipFirst = false;
  _gzipStats = 0xF8;
}

AsyncStaticSDWebHandler& AsyncStaticSDWebHandler::setIsDir(bool isDir){
  _isDir = isDir;
  return *this;
}

AsyncStaticSDWebHandler& AsyncStaticSDWebHandler::setDefaultFile(const char* filename){
  _default_file = String(filename);
  return *this;
}

AsyncStaticSDWebHandler& AsyncStaticSDWebHandler::setCacheControl(const char* cache_control){
  _cache_control = String(cache_control);
  return *this;
}

AsyncStaticSDWebHandler& AsyncStaticSDWebHandler::setLastModified(const char* last_modified){
  _last_modified = String(last_modified);
  return *this;
}

AsyncStaticSDWebHandler& AsyncStaticSDWebHandler::setLastModified(struct tm* last_modified){
  char result[30];
  strftime (result,30,"%a, %d %b %Y %H:%M:%S %Z", last_modified);
  return setLastModified((const char *)result);
}
#ifdef ESP8266
AsyncStaticSDWebHandler& AsyncStaticSDWebHandler::setLastModified(time_t last_modified){
  return setLastModified((struct tm *)gmtime(&last_modified));
}

AsyncStaticSDWebHandler& AsyncStaticSDWebHandler::setLastModified(){
  time_t last_modified;
  if(time(&last_modified) == 0) //time is not yet set
    return *this;
  return setLastModified(last_modified);
}
#endif
bool AsyncStaticSDWebHandler::canHandle(AsyncWebServerRequest *request){
  if (request->method() == HTTP_GET &&
      request->url().startsWith(_uri) &&
      _getFile(request)) {

    // We interested in "If-Modified-Since" header to check if file was modified
    if (_last_modified.length())
      request->addInterestingHeader("If-Modified-Since");

    if(_cache_control.length())
      request->addInterestingHeader("If-None-Match");

    DEBUGF("[AsyncStaticSDWebHandler::canHandle] TRUE\n");
    return true;
  }

  return false;
}

bool AsyncStaticSDWebHandler::_getFile(AsyncWebServerRequest *request)
{
  // Remove the found uri
  String path = request->url().substring(_uri.length());

  // We can skip the file check and look for default if request is to the root of a directory or that request path ends with '/'
  bool canSkipFileCheck = (_isDir && path.length() == 0) || (path.length() && path[path.length()-1] == '/');

  path = _path + path;

  // Do we have a file or .gz file
  if (!canSkipFileCheck && _fileExists(request, path))
    return true;

  // Can't handle if not default file
  if (_default_file.length() == 0)
    return false;

  // Try to add default file, ensure there is a trailing '/' ot the path.
  if (path.length() == 0 || path[path.length()-1] != '/')
    path += "/";
  path += _default_file;

  return _fileExists(request, path);
}

bool AsyncStaticSDWebHandler::_fileExists(AsyncWebServerRequest *request, const String& path)
{
  bool fileFound = false;
  bool gzipFound = false;

  String gzip = path + ".gz";

  // Following part reworked to use SD_exists instead of request->_tempFile = _fs.open()
  // Drawback: AsyncSDFileResponse(sd::File content, ...) cannot be used so
  //            file needs to be opened again in AsyncSDFileResponse(SDClass &sd, ...)
  //            request->_tempFile is of wrong fs::File type anyway...
  if (_gzipFirst) {
    gzipFound = SD_exists(_sd, gzip);
    if (!gzipFound){
      fileFound = SD_exists(_sd, path);
    }
  } else {
    fileFound = SD_exists(_sd, path);
    if (!fileFound){
      gzipFound = SD_exists(_sd, gzip);
    }
  }

  bool found = fileFound || gzipFound;

  if (found) {
    // Extract the file name from the path and keep it in _tempObject
    size_t pathLen = path.length();
    char * _tempPath = (char*)malloc(pathLen+1);
    snprintf(_tempPath, pathLen+1, "%s", path.c_str());
    request->_tempObject = (void*)_tempPath;

    // Calculate gzip statistic
    _gzipStats = (_gzipStats << 1) + (gzipFound ? 1 : 0);
    if (_gzipStats == 0x00) _gzipFirst = false; // All files are not gzip
    else if (_gzipStats == 0xFF) _gzipFirst = true; // All files are gzip
    else _gzipFirst = _countBits(_gzipStats) > 4; // IF we have more gzip files - try gzip first
  }

  return found;
}

uint8_t AsyncStaticSDWebHandler::_countBits(const uint8_t value) const
{
  uint8_t w = value;
  uint8_t n;
  for (n=0; w!=0; n++) w&=w-1;
  return n;
}

void AsyncStaticSDWebHandler::handleRequest(AsyncWebServerRequest *request)
{
  // Get the filename from request->_tempObject and free it
  String filename = String((char*)request->_tempObject);
  free(request->_tempObject);
  request->_tempObject = NULL;

  File _tempFile = _sd.open(filename);
  if (_tempFile == true) {
    String etag = String(_tempFile.size());
    _tempFile.close();    
    if (_last_modified.length() && _last_modified == request->header("If-Modified-Since")) {
      request->send(304); // Not modified
    } else if (_cache_control.length() && request->hasHeader("If-None-Match") && request->header("If-None-Match").equals(etag)) {
      AsyncWebServerResponse * response = new AsyncBasicResponse(304); // Not modified
      response->addHeader("Cache-Control", _cache_control);
      response->addHeader("ETag", etag);
      request->send(response);
    } else {
      // Cannot use new AsyncSDFileResponse(request->_tempFile, ...) here because request->_tempFile has not be opened
      // in AsyncStaticSDWebHandler::_fileExists and is of wrong type fs::File anyway.
      AsyncWebServerResponse * response = new AsyncSDFileResponse(/* request->_tempFile, */ _sd, filename);
      if (_last_modified.length())
        response->addHeader("Last-Modified", _last_modified);
      if (_cache_control.length()){
        response->addHeader("Cache-Control", _cache_control);
        response->addHeader("ETag", etag);
      }
      request->send(response);
    }
  } else {
    request->send(404);
  }
}

/* 
 *  We don't need this one
AsyncStaticSDWebHandler& AsyncWebServer::serveStatic(const char* uri, SDClass& sd, const char* path, const char* cache_control){
  AsyncStaticSDWebHandler* handler = new AsyncStaticSDWebHandler(uri, sd, path, cache_control);
  addHandler(handler);
  return *handler;
}
*/

extern "C" void system_set_os_print(uint8 onoff);
extern "C" void ets_install_putc1(void* routine);

static void _u0_putc(char c){
  while(((U0S >> USTXC) & 0x7F) != 0);
  U0F = c;
}
