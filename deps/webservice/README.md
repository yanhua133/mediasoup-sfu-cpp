# webservice

A C++14 boost.beast based HTTP and WebSocket server and a WebSocket client.

## Interface

### Class server

If you want an HTTP or WebSocket server you need a server object. The server
class takes an HTTP handler and a WebSocket service.

If yout don't need the HTTP part, pass a `nullptr` as request handler. This
will create a `http_request_handler` object which will answer all plain HTTP
requests with status code not-found. In the most cases you want a
`file_request_handler` instead. It takes a directory as argument and sends the
files from there. If you need another kind of http request handler derive from
`http_request_handler` and write your own virtual function operator to handle
a request.

For WebSocket services you always need to write your own service derived from
`ws_handler_base`. `ws_handler` is derived from it and handles text messages as
`std::string` and binary messages as `std::vector< std::uint64_t >`. If you
want other types, derive from `basic_ws_handler` and set the template
parameters to your wanted types. If you don't need a WebSocket server pass
a `nullptr`.

The `ws_client` class is simmiar to `ws_handler` on the server side.

For server error handling purpose you can pass an `error_handler` derived
object or a `nullptr` if you wish to ignore errors and execptions in the
server.

### Server interface `ws_handler` and client interface `ws_client`

Use `send_text` and `send_binary` to send messages. Derive from the classes to
implement your own handler functions `on_open`, `on_close`, `on_text` and
`on_binary`.

A `ws_client` has always only one session while a `ws_handler` must handler
multiple sessions. Therefore the `ws_handler` functions get and take an
additional parameter identifier which is unique per session. The handler
functions take another additional parameter resource which is the request
target.

If you need more than one kind of WebSocket service, you can use a
`ws_service_handler` object and add `ws_handler_base` derived services to it.
The http resource maps to it. You should derive your added service handling
objects from `ws_service` or `basic_ws_service` which are equivalent to
`ws_handler` and `basic_ws_handler`, except for the missing resource parameter.

### WebSocket timeouts and read message limits

Both `server` and `ws_client` support the parameters `websocket_ping_time` and
`max_read_message_size`.

`websocket_ping_time` is by default an empty `boost::optional` which means that
there are no timeouts. If you pass a time in millisecond every WebSocket
session has an activity timer which traks incomming messages. It restarts after
every incomming message. If it expires the first time, a ping message is sent.
The connection is closed on socket layer if it expires a second time in
succession.

`max_read_message_size` is as the name indicates the maximun count of bytes a
received message is allowed to have. By default it is `16 MiB`. Set it to `0`
if you want no limit.

### Error and exception handling

All classes with virtual handler functions have also a virtual `on_error` and
`on_exception` function. `on_error` is called when an async function failes
with an `error_code`. `on_exception` is called when an exception occours in one
of the virtual handler functions (inclusive `on_error`) and must not `throw`
itself. (`noexcept`)

`on_error` takes an class specific `enum` descriping the location of the error
and and `boost::system::error_code` descriping the error itself.

`on_exception` takes an `std::exception_ptr` as agument.

The `error_handler` functions are called for errors in the server class and
exceptions in the webservice libraries implementation.

## Known problems

### WebSocket: broken pipe after timeout

If a user defined handler like `on_text` or `on_binary` takes to long time, the
activity timer expires and closes the session with a timeout. The session can
not pong while a handler is running. Set `websocket_ping_time` to a higher
value or to `0` or implement you handling asynchronous.

## License notice

This software was originally developed privately by Benjamin Buch. All changes are released under the Boost Software License - Version 1.0 and published on GitHub.

https://github.com/bebuch/webservice
