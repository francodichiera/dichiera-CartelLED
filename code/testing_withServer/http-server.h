#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <FS.h>
#include <uri/UriBraces.h>

class HttpServer {
public:

  HttpServer();

  void begin(uint16_t port = 80);

  void handleClient();

  void on(const String &uri, std::function<void()> handler);
  void on(const UriBraces &uri, std::function<void()> handler);


  void send(int code, const String &content_type, const String &content);
  void send(int code, const char *content_type, const char *content);
  void send(int code, const char *content_type, const String &content);

  String arg(const String &name);
  String method();
  String uri();

  String getUrlParam(int index);
  void sendHeader(const String &name, const String &value);

  size_t streamFile(File &file, const String &contentType);
  size_t streamFile(File &file, const char *contentType);

private:
  WebServer *server;
};

#endif
