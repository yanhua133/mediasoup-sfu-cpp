# oatpp-libressl [![oatpp build status](https://dev.azure.com/lganzzzo/lganzzzo/_apis/build/status/oatpp.oatpp-libressl)](https://dev.azure.com/lganzzzo/lganzzzo/_build?definitionId=3)
This submodule provides secure server and client connection providers for oatpp applications. Based on LibreSSL.

More about oat++:
- Website: [https://oatpp.io](https://oatpp.io)

## Requires

LibreSSL installed.

## Example

See: [Full example project TLS-Libressl](https://github.com/oatpp/example-libressl)

### Create server connection provider

```c++

#include "oatpp-libressl/server/ConnectionProvider.hpp"
#include "oatpp-libressl/Config.hpp"

...

const char* pemFile = "path/to/file.pem";
const char* crtFile = "path/to/file.crt";

auto config = oatpp::libressl::Config::createDefaultServerConfig(pemFile, crtFile);
auto connectionProvider = oatpp::libressl::server::ConnectionProvider::createShared(config, 8443);

```

### Create client connection provider

```c++

#include "oatpp-libressl/client/ConnectionProvider.hpp"
#include "oatpp-libressl/Config.hpp"

...

auto config = oatpp::libressl::Config::createShared();
auto connectionProvider = oatpp::libressl::client::ConnectionProvider::createShared(config, "httpbin.org", 443);

```

## Don't forget!

Set libressl lockingCallback and SIGPIPE handler on program start!

```c++

#include "oatpp-libressl/Callbacks.hpp"

...

/* set lockingCallback for libressl */
oatpp::libressl::Callbacks::setDefaultCallbacks();

```

```c++
#include <csignal>

...

/* ignore SIGPIPE */
std::signal(SIGPIPE, SIG_IGN);
```
