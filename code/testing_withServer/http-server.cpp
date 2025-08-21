#include "http-server.h"

HttpServer::HttpServer() : server(nullptr) {}

void HttpServer::begin(uint16_t port)
{
    if (server != nullptr)
    {
        delete server;
    }

    server = new WebServer(port);

    server->begin();
    Serial.println("Servidor HTTP iniciado");
}

void HttpServer::handleClient()
{
    if (server != nullptr)
    {
        server->handleClient();
    }
}

void HttpServer::on(const String &uri, std::function<void()> handler)
{
    if (server != nullptr)
    {
        server->on(uri, handler);
    }
}

void HttpServer::on(const UriBraces &uri, std::function<void()> handler)
{
    if (server != nullptr)
    {
        server->on(uri, handler);
    }
}

void HttpServer::send(int code, const String &content_type, const String &content)
{
    if (server != nullptr)
    {
        server->send(code, content_type, content);
    }
}

void HttpServer::send(int code, const char *content_type, const char *content)
{
    if (server != nullptr)
    {
        server->send(code, content_type, content);
    }
}

void HttpServer::send(int code, const char *content_type, const String &content)
{
    if (server != nullptr)
    {
        server->send(code, content_type, content);
    }
}

String HttpServer::arg(const String &name)
{
    if (server != nullptr)
    {
        return server->arg(name);
    }
    return String();
}

String HttpServer::method()
{
    if (server != nullptr)
    {
        // Convertir HTTPMethod a String
        switch (server->method())
        {
        case HTTP_GET:
            return "GET";
        case HTTP_POST:
            return "POST";
        case HTTP_PUT:
            return "PUT";
        case HTTP_DELETE:
            return "DELETE";
        case HTTP_PATCH:
            return "PATCH";
        case HTTP_OPTIONS:
            return "OPTIONS";
        default:
            return "UNKNOWN";
        }
    }
    return String();
}

String HttpServer::uri()
{
    if (server != nullptr)
    {
        return server->uri();
    }
    return String();
}

String HttpServer::getUrlParam(int index)
{
    if (server != nullptr)
    {
        return server->pathArg(index);
    }
    return String();
}

void HttpServer::sendHeader(const String &name, const String &value)
{
    if (server != nullptr)
    {
        server->sendHeader(name, value);
    }
}

size_t HttpServer::streamFile(File &file, const String &contentType)
{
    return server->streamFile(file, contentType);
}

size_t HttpServer::streamFile(File &file, const char *contentType)
{
    return server->streamFile(file, contentType);
}
