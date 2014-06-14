// Copyright 2013 by Institute of Formal and Applied Linguistics, Faculty of
// Mathematics and Physics, Charles University in Prague, Czech Republic.
// All rights reserved.

#include <cctype>
#include <cstdarg>
#include <cstring>
#include <ctime>
#include <regex.h>

#include "MicrohttpdServer.h"
#include "ResponseGenerator.h"

namespace ngramchecker {

class MHD_ResponseDeleter {
 public:
  void operator()(MHD_Response* response) {
    MHD_destroy_response(response);
  }
};

class MHD_PostProcessorDeleter {
 public:
  void operator()(MHD_PostProcessor* post_processor) {
    MHD_destroy_post_processor(post_processor);
  }
};

class MicrohttpdRequest : public RestRequest {
 public:
  static bool initialize();

  MicrohttpdRequest(MHD_Connection* connection, const char* url, const char* method, unsigned max_post_size);

  int handle(RestService* service);
  bool process_post_data(const char* post_data, size_t post_data_len);

  virtual bool respond_json(StringPiece json) override;
  virtual bool respond_json(ResponseGenerator* generator) override;
  virtual bool respond_not_found() override;

 private:
  MHD_Connection* connection;
  unique_ptr<MHD_PostProcessor, MHD_PostProcessorDeleter> post_processor;
  unique_ptr<ResponseGenerator> generator;
  string method;
  unsigned remaining_post_limit;
  bool unsupported_post_data;

  static MHD_Response* create_permanent_response(const string& data);
  static MHD_Response* create_response(StringPiece data);
  static MHD_Response* create_generator_response(ResponseGenerator* generator);

  static int key_value_iterator(void* cls, MHD_ValueKind kind, const char* key, const char* value);
  static int post_iterator(void* cls, MHD_ValueKind kind, const char* key, const char* filename, const char* content_type, const char* transfer_encoding, const char* data, uint64_t off, size_t size);
  static ssize_t generator_callback(void* cls, uint64_t pos, char* buf, size_t max);

  static bool valid_utf8(const string& text);

  static unique_ptr<MHD_Response, MHD_ResponseDeleter> response_not_allowed, response_not_found, response_too_large, response_unsupported_post_data, response_invalid_utf8;
  static regex_t supported_content_type, supported_transfer_encoding;
};
unique_ptr<MHD_Response, MHD_ResponseDeleter> MicrohttpdRequest::response_not_allowed, MicrohttpdRequest::response_not_found, MicrohttpdRequest::response_too_large, MicrohttpdRequest::response_unsupported_post_data, MicrohttpdRequest::response_invalid_utf8;
regex_t MicrohttpdRequest::supported_content_type, MicrohttpdRequest::supported_transfer_encoding;

MicrohttpdRequest::MicrohttpdRequest(MHD_Connection* connection, const char* url, const char* method, unsigned max_post_size)
  : RestRequest(url), connection(connection), method(method), remaining_post_limit(max_post_size + 1), unsupported_post_data(false) {
  if (strcmp(method, MHD_HTTP_METHOD_POST) == 0) {
    post_processor.reset(MHD_create_post_processor(connection, 1<<15, &post_iterator, this));
    if (!post_processor) fprintf(stderr, "Cannot allocate new post processor!\n");
  }
}

bool MicrohttpdRequest::initialize() {
  static string not_allowed = "Requested method is not allowed.\r\n";
  static string not_found = "Requested URL was not found.\r\n";
  static string too_large = "Request was too large.\r\n";
  static string unsupported_post_data = "Unsupported format of the multipart/form-data POST request. Currently only the following is supported:\r\n"
      " - Content-type: application/octet-stream or text/plain or text/plain; charset=utf-8\r\n"
      " - Content-transfer-encoding: 7bit or 8bit or binary\r\n";
  static string invalid_utf8 = "The GET or POST arguments are not valid UTF-8.\r\n";

  response_not_allowed.reset(create_permanent_response(not_allowed));
  if (!response_not_allowed) return false;
  if (MHD_add_response_header(response_not_allowed.get(), MHD_HTTP_HEADER_ALLOW, "HEAD, GET, POST") != MHD_YES) return false;

  response_not_found.reset(create_permanent_response(not_found));
  if (!response_not_found) return false;

  response_too_large.reset(create_permanent_response(too_large));
  if (!response_too_large) return false;

  response_unsupported_post_data.reset(create_permanent_response(unsupported_post_data));
  if (!response_unsupported_post_data) return false;

  response_invalid_utf8.reset(create_permanent_response(invalid_utf8));
  if (!response_invalid_utf8) return false;

  if (regcomp(&supported_content_type, "^[[:space:]]*((text/plain|application/octet-stream)[[:space:]]*(;[[:space:]]*(charset[[:space:]]*=[[:space:]]*utf-8[[:space:]]*)?)?)?$", REG_EXTENDED | REG_ICASE | REG_NOSUB)) return false;
  if (regcomp(&supported_transfer_encoding, "^[[:space:]]*((binary|[78]bit)[[:space:]]*)?$", REG_EXTENDED | REG_ICASE | REG_NOSUB)) return false;

  return true;
}

int MicrohttpdRequest::handle(RestService* service) {
  // Check that method is supported
  if (method != MHD_HTTP_METHOD_HEAD && method != MHD_HTTP_METHOD_GET && method != MHD_HTTP_METHOD_POST)
    return MHD_queue_response(connection, MHD_HTTP_METHOD_NOT_ALLOWED, response_not_allowed.get());

  // Close post_processor if exists
  if (post_processor) post_processor.reset();

  // Collect initial POST and GET arguments
  MHD_get_connection_values(connection, MHD_POSTDATA_KIND, key_value_iterator, this);
  MHD_get_connection_values(connection, MHD_GET_ARGUMENT_KIND, key_value_iterator, this);

  // Was the POST format supported
  if (unsupported_post_data)
    return MHD_queue_response(connection, MHD_HTTP_UNSUPPORTED_MEDIA_TYPE, response_unsupported_post_data.get());

  // Was the request too large?
  if (!remaining_post_limit)
    return MHD_queue_response(connection, MHD_HTTP_REQUEST_ENTITY_TOO_LARGE, response_too_large.get());

  // Are all arguments legal utf-8?
  for (auto&& param : params)
    if (!valid_utf8(param.first) || !valid_utf8(param.second))
      return MHD_queue_response(connection, MHD_HTTP_UNSUPPORTED_MEDIA_TYPE, response_invalid_utf8.get());

  // Let the service handle the request and respond with one of the respond_* methods.
  return service->handle(*this) ? MHD_YES : MHD_NO;
}

bool MicrohttpdRequest::process_post_data(const char* post_data, size_t post_data_len) {
  return post_processor && MHD_post_process(post_processor.get(), post_data, post_data_len) == MHD_YES;
}

bool MicrohttpdRequest::respond_json(StringPiece json) {
  unique_ptr<MHD_Response, MHD_ResponseDeleter> response(create_response(json));
  if (!response) return false;
  if (MHD_add_response_header(response.get(), MHD_HTTP_HEADER_CONTENT_TYPE, "application/json") != MHD_YES) return false;
  if (MHD_add_response_header(response.get(), MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*") != MHD_YES) return false;
  return MHD_queue_response(connection, MHD_HTTP_OK, response.get()) == MHD_YES;
}

bool MicrohttpdRequest::respond_json(ResponseGenerator* generator) {
  this->generator.reset(generator);
  unique_ptr<MHD_Response, MHD_ResponseDeleter> response(create_generator_response(generator));
  if (!response) return false;
  if (MHD_add_response_header(response.get(), MHD_HTTP_HEADER_CONTENT_TYPE, "application/json") != MHD_YES) return false;
  if (MHD_add_response_header(response.get(), MHD_HTTP_HEADER_ACCESS_CONTROL_ALLOW_ORIGIN, "*") != MHD_YES) return false;
  return MHD_queue_response(connection, MHD_HTTP_OK, response.get()) == MHD_YES;
}

bool MicrohttpdRequest::respond_not_found() {
  return MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response_not_found.get());
}

MHD_Response* MicrohttpdRequest::create_permanent_response(const string& data) {
  unique_ptr<MHD_Response, MHD_ResponseDeleter> response(MHD_create_response_from_buffer(data.size(), (void*) data.c_str(), MHD_RESPMEM_PERSISTENT));
  if (response && MHD_add_response_header(response.get(), MHD_HTTP_HEADER_CONNECTION, "close") != MHD_YES)
    response.reset();
  return response.release();
}

MHD_Response* MicrohttpdRequest::create_response(StringPiece data) {
  unique_ptr<MHD_Response, MHD_ResponseDeleter> response(MHD_create_response_from_buffer(data.len, (void*) data.str, MHD_RESPMEM_MUST_COPY));
  if (response && MHD_add_response_header(response.get(), MHD_HTTP_HEADER_CONNECTION, "close") != MHD_YES)
    response.reset();
  return response.release();
}

MHD_Response* MicrohttpdRequest::create_generator_response(ResponseGenerator* generator) {
  unique_ptr<MHD_Response, MHD_ResponseDeleter> response(MHD_create_response_from_callback(-1, 1024, generator_callback, generator, nullptr));
  if (response && MHD_add_response_header(response.get(), MHD_HTTP_HEADER_CONNECTION, "close") != MHD_YES)
    response.reset();
  return response.release();
}


int MicrohttpdRequest::key_value_iterator(void* cls, MHD_ValueKind kind, const char* key, const char* value) {
  auto self = (MicrohttpdRequest*) cls;
  if ((kind == MHD_POSTDATA_KIND || kind == MHD_GET_ARGUMENT_KIND) && self->remaining_post_limit) {
    auto value_len = value ? strlen(value) : 0;
    if (self->remaining_post_limit > value_len) {
      if (self->params.emplace(key, value ? value : string()).second)
        self->remaining_post_limit -= value_len;
    } else {
      self->remaining_post_limit = 0;
    }
  }
  return MHD_YES;
}

int MicrohttpdRequest::post_iterator(void* cls, MHD_ValueKind kind, const char* key, const char* /*filename*/, const char* content_type, const char* transfer_encoding, const char* data, uint64_t off, size_t size) {
  auto self = (MicrohttpdRequest*) cls;
  if (kind == MHD_POSTDATA_KIND && self->remaining_post_limit) {
    // Check that content_type and transfer_encoding are supported
    if ((content_type && regexec(&supported_content_type, content_type, 0, nullptr, 0) != 0) ||
        (transfer_encoding && regexec(&supported_transfer_encoding, transfer_encoding, 0, nullptr, 0) != 0)) {
      self->unsupported_post_data = true;
      self->remaining_post_limit = 0;
    }

    if (self->remaining_post_limit > size) {
      string& value = self->params[key];
      if (!off) value.clear();
      if (value.size() == off) {
        if (size) value.append(data, size);
        self->remaining_post_limit -= size;
      } else {
        fprintf(stderr, "Cannot append to key %s at offset %u, have only %u\n", key, unsigned(off), unsigned(value.size()));
      }
    } else {
      self->remaining_post_limit = 0;
    }
  }

  return MHD_YES;
}

ssize_t MicrohttpdRequest::generator_callback(void* cls, uint64_t /*pos*/, char* buf, size_t max) {
  auto generator = (ResponseGenerator*) cls;
  bool end = false;
  StringPiece data;
  while (!end && (data = generator->current()).len < max)
    end = !generator->generate();

  // End of data?
  if (end && !data.len) return MHD_CONTENT_READER_END_OF_STREAM;

  // Copy generated data and remove them from the generator
  size_t data_len = min(data.len, max);
  memcpy(buf, data.str, data_len);
  generator->consume(data_len);
  return data_len;
}

bool MicrohttpdRequest::valid_utf8(const string& text) {
  for (auto str = (const unsigned char*) text.c_str(); *str; str++)
    if (*str >= 0x80) {
      if (*str < 0xC0) return false;
      else if (*str < 0xE0) {
        str++; if (*str < 0x80 || *str >= 0xC0) return false;
      } else if (*str < 0xF0) {
        str++; if (*str < 0x80 || *str >= 0xC0) return false;
        str++; if (*str < 0x80 || *str >= 0xC0) return false;
      } else if (*str < 0xF8) {
        str++; if (*str < 0x80 || *str >= 0xC0) return false;
        str++; if (*str < 0x80 || *str >= 0xC0) return false;
        str++; if (*str < 0x80 || *str >= 0xC0) return false;
      } else return false;
    }

  return true;
}

// MicrohttpdServer
bool MicrohttpdServer::start(RestService* service, unsigned port, unsigned max_connections, unsigned timeout, unsigned max_post_size) {
  if (!service) return false;

  this->service = service;
  this->max_post_size = max_post_size;

  if (!MicrohttpdRequest::initialize()) return false;

  daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION | MHD_USE_POLL, port, nullptr, nullptr, &handle_request, this,
                            MHD_OPTION_CONNECTION_LIMIT, max_connections,
                            MHD_OPTION_CONNECTION_MEMORY_LIMIT, size_t(64 * 1024),
                            MHD_OPTION_CONNECTION_TIMEOUT, timeout,
                            MHD_OPTION_NOTIFY_COMPLETED, &request_completed, this,
                            MHD_OPTION_END);

  return daemon;
}

int MicrohttpdServer::handle_request(void* cls, struct MHD_Connection* connection, const char* url, const char* method, const char* /*version*/, const char* upload_data, size_t* upload_data_size, void** con_cls) {
  auto self = (MicrohttpdServer*) cls;
  auto request = (MicrohttpdRequest*) *con_cls;

  // Do we have a new request?
  if (!request) {
    if (!(request = new MicrohttpdRequest(connection, url, method, self->max_post_size)))
      return fprintf(stderr, "Cannot allocate new request!\n"), MHD_NO;

    *con_cls = request;
    return MHD_YES;
  }

  // Are we processing POST data?
  if (*upload_data_size) {
    if (request->process_post_data(upload_data, *upload_data_size) != MHD_YES)
      return MHD_NO;

    *upload_data_size = 0;
    return MHD_YES;
  }

  // Handle complete request
  return request->handle(self->service);
}

void MicrohttpdServer::request_completed(void* /*cls*/, struct MHD_Connection* /*connection*/, void** con_cls, MHD_RequestTerminationCode /*toe*/) {
  auto request = (MicrohttpdRequest*) *con_cls;
  if (request) delete request;
}

} // namespace ngramchecker
